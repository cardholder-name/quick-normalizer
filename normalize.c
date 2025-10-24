/*
	normalize.c - main source file for PCM WAV normalizer - v1.0.1
	
	Original normalize v0.253:
	(c) 2000-2004 Manuel Kasper <mk@neon1.net>
	smartpeak code by Lapo Luchini <lapo@lapo.it>
	
	LUFS Normalization & Watch Mode additions (2025):
	Cam St Clair with assistance from Claude (Anthropic)
	- ITU-R BS.1770-4 compliant LUFS normalization
	- Automated folder watching for unattended processing
	- Enhanced file handling and error recovery

	This file is part of normalize.

	normalize is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	normalize is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "pcmwav.h"

#define COPYRIGHT_NOTICE	"normalize v1.0.1 (c) 2000-2004 Manuel Kasper <mk@neon1.net>.\n" \
							"All rights reserved.\n" \
							"smartpeak code by Lapo Luchini <lapo@lapo.it>.\n" \
							"LUFS support and watch mode added 2025 by Cam St Clair with Claude (Anthropic)."

// Biquad filter structure for K-weighting
typedef struct {
	double b0, b1, b2;  // Numerator coefficients
	double a1, a2;      // Denominator coefficients (a0 is always 1)
	double z1, z2;      // State variables
} biquad_filter;

// K-weighting filter pair (ITU-R BS.1770-4)
typedef struct {
	biquad_filter shelf;     // High-shelf pre-filter (~4kHz, +4dB)
	biquad_filter highpass;  // High-pass RLB filter (~38Hz)
} k_weighting;

void			*buf;
unsigned char	*buf8;
unsigned short	*buf16;
signed char		*table8;
signed short	*table16;
unsigned long	iobufsize = 65536;
pcmwavfile		pwf;
HANDLE			outf;
double			ratio, normpercent = 100.0, peakpercent = 100.0;
int				smartpeak = 0;
double			mingain = 0;
int				usemingain = 0;
int				quiet = 0, nooverwrite = 0;
char			outfname[1024];
int				dowhat = 0, prompt = 0;
int				dontabort = 0;
int				lufs_mode = 0;
double			target_lufs = -16.0;
double			gate_percentile = 100.0;
int				watch_mode = 0;
char			watch_folder[1024];
char			output_folder[1024];

void make_table8(void);
void make_table16(void);
int getpeaks8(signed char *minpeak, signed char *maxpeak);
int getpeaks16(signed short *minpeak, signed short *maxpeak);
unsigned long amplify8(void);
unsigned long amplify16(void);
unsigned long passthrough(void);
int process_filespec(char *fspec);
int process_file(char *fname);
void usage(void);
void init_k_weighting(k_weighting *kw, unsigned long samplerate);
double apply_k_weighting(k_weighting *kw, double sample);
double calculate_lufs8(void);
double calculate_lufs16(void);
int compare_double(const void *a, const void *b);
void process_existing_files(char *folder, char *outfolder);
int watch_folder_mode(char *folder, char *outfolder);
int is_file_ready(char *filepath);
int move_file_to_output(char *srcpath, char *outfolder);

int main(int argc, char *argv[]) {

	int				i;
	
	/* Parse command line */
	for (i = 1; i < argc; i++) {
		if ((argv[i][0] == '-') && (argv[i][1] != 0x00)) {
			switch (argv[i][1]) {
				case 'h':
					usage();
					return 0;
				case 'q':
					quiet = 1;
					break;
				case 'd':
					dontabort = 1;
					break;
				case 'p':
					prompt = 1;
					break;
				case 'm':
					normpercent = atof(argv[++i]);
					break;
				case 's':
					peakpercent = atof(argv[++i]);
					if (peakpercent < 50.0)
						peakpercent = 50.0;
					if (peakpercent < 100.0)
						smartpeak = 1;
					break;
				case 'o':
					nooverwrite = 1;
					strcpy(outfname, argv[++i]);
					break;
				case 'x':
					mingain = atof(argv[++i]);
					usemingain = 1;
					break;
				case 'l':
					if (dowhat != 0) {
						fprintf(stderr, "You can't specify both -l and -a. Aborting.\n");
						return 2;
					} else {
						dowhat = 1;
						ratio = atof(argv[++i]);
					}
					break;
				case 'a':
					if (dowhat != 0) {
						fprintf(stderr, "You can't specify both -l and -a. Aborting.\n");
						return 2;
					} else {
						dowhat = 2;
						ratio = pow(10, atof(argv[++i]) / 20);
					}
					break;
				case 'b':
					iobufsize = atoi(argv[++i]) * 1024;
					if ((iobufsize < 16384) || (iobufsize > 16777216)) {
						fprintf(stderr, "I/O buffer size must be between 16 and 16384 KB.\n");
						return 2;
					}
					break;
				case 'L':
					if (dowhat != 0) {
						fprintf(stderr, "You can't specify -L with -l or -a. Aborting.\n");
						return 2;
					}
					lufs_mode = 1;
					dowhat = 3;
					target_lufs = atof(argv[++i]);
					break;
				case 'g':
					gate_percentile = atof(argv[++i]);
					if (gate_percentile < 50.0)
						gate_percentile = 50.0;
					if (gate_percentile > 100.0)
						gate_percentile = 100.0;
					break;
				case 'w':
					watch_mode = 1;
					strcpy(watch_folder, argv[++i]);
					break;
				case 'O':
					strcpy(output_folder, argv[++i]);
					break;
				default:
					fprintf(stderr, "Error: Can't understand flag -%c. Aborting.\n", argv[i][1]);
					return 2;
				}
		} else {
			break;
		}
	}

	// this way the percentile peak is amplified to the correct level
	if (smartpeak)
		normpercent *= peakpercent / 100.0;

	// Handle watch mode
	if (watch_mode) {
		if (output_folder[0] == '\0') {
			fprintf(stderr, "Error: Watch mode requires -O <output-folder>. Aborting.\n");
			return 2;
		}
		
		if (!quiet) {
			fprintf(stderr, "\n%s\n\n", COPYRIGHT_NOTICE);
			fprintf(stderr, "Watch mode activated.\n");
			fprintf(stderr, "Watching folder: %s\n", watch_folder);
			fprintf(stderr, "Output folder: %s\n", output_folder);
			fprintf(stderr, "Press Ctrl+C to stop...\n\n");
		}
		
		return watch_folder_mode(watch_folder, output_folder);
	}

	if (i >= argc) {
		usage();
		return 2;
	}

	if (!quiet)
		fprintf(stderr, "\n%s\n\n", COPYRIGHT_NOTICE);

	return process_filespec(argv[i]);

	return 0;
}

int process_filespec(char *fspec) {
	long	hFile;
	char	myfullpath[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	struct	_finddata_t my_file;
	int		err;

	_fullpath(myfullpath, fspec, _MAX_PATH);
	_splitpath(myfullpath, drive, dir, NULL, NULL);

	if ((hFile = _findfirst(fspec, &my_file)) == -1L)
		fprintf(stderr, "Could not find file %s.\n", fspec);
	else {
		
		do {

			if (my_file.attrib & _A_SUBDIR)
				continue;

			sprintf(myfullpath, "%s%s%s", drive, dir, my_file.name);
			
			err = process_file(myfullpath);
			if (err && (err != 3)) {
				
				if ((err != 5) || !dontabort)
					return err;
			}

		} while (_findnext(hFile, &my_file) == 0);

		_findclose(hFile);
	}

	return err;
}

int process_file(char *fname) {

	clock_t		sclk, eclk;
	double		atime;
	unsigned long	ndata;

	if (!quiet) {
		
		fprintf(stderr, "-------------------------------------------------------------------------------\n");
		fprintf(stderr, "Processing file %s\n\n", fname);
	}

	// Open PCM WAV file
	if (!pcmwav_open(fname, GENERIC_READ | GENERIC_WRITE, &pwf)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 1;
	}

	if (nooverwrite) {
		char	hdrbuf[16384];
		DWORD	nread;

		outf = CreateFile(outfname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (outf == INVALID_HANDLE_VALUE) {
			if (!quiet)
				fprintf(stderr, "Couldn't open output file '%s'.\n", outfname);
			return 1;
		}
		// Copy headers
		SetFilePointer(pwf.winfile, 0, NULL, FILE_BEGIN);
		ReadFile(pwf.winfile, hdrbuf, pwf.datapos, &nread, NULL);
		if (nread != pwf.datapos) {
			if (!quiet)
				fprintf(stderr, "Could not copy headers.\n");
			return 1;
		}
		SetFilePointer(pwf.winfile, pwf.datapos, NULL, FILE_BEGIN);
		WriteFile(outf, hdrbuf, pwf.datapos, &nread, NULL);
		if (nread != pwf.datapos) {
			if (!quiet)
				fprintf(stderr, "Could not copy headers.\n");
			return 1;
		}
	}

	// Allocate buffer
	buf = VirtualAlloc(NULL, iobufsize, MEM_COMMIT, PAGE_READWRITE);

	if (buf == NULL) {
		if (!quiet)
			fprintf(stderr, "Cannot allocate buffer in memory.\n");
		return 1;
	}

	buf8 = (signed char*)buf;
	buf16 = (signed short*)buf;

	if (dowhat == 0) {
		if (pwf.bitspersample == 8) {
			signed char	mins, maxs;

			if (!quiet)
				fprintf(stderr, "Pass 1: Finding peak levels...\n");

			if (!getpeaks8(&mins, &maxs))
				return 1;

			if (!quiet)
				fprintf(stderr, "\rMinimum level found: %d, maximum level found: %d\n", (short)mins, (short)maxs);

			if (mins == -128)
				mins = -127;

			if ((-mins) > maxs)
				maxs = -mins;

			if (maxs == 0) {
				if (!quiet)
					fprintf(stderr, "All zero samples found.\n");
				ratio = 1;
			} else {
				ratio = (127.0 * normpercent) / ((double)maxs * 100.0);
			}

		} else if (pwf.bitspersample == 16) {
			signed short	mins, maxs;

			if (!quiet)
				fprintf(stderr, "Pass 1: Finding peak levels...\n");

			if (!getpeaks16(&mins, &maxs))
				return 1;

			if (!quiet)
				fprintf(stderr, "\rMinimum level found: %d, maximum level found: %d\n", mins, maxs);

			if (mins == -32768)
				mins = -32767;

			if ((-mins) > maxs)
				maxs = -mins;

			if (maxs == 0) {
				if (!quiet)
					fprintf(stderr, "All zero samples found.\n");
				ratio = 1;
			} else {
				ratio = (32767.0 * normpercent) / ((double)maxs * 100.0);
			}
		}
	} else if (dowhat == 3) {
		// LUFS normalization mode
		double measured_lufs;
		
		if (!quiet)
			fprintf(stderr, "Pass 1: Calculating LUFS loudness...\n");
		
		if (pwf.bitspersample == 8) {
			measured_lufs = calculate_lufs8();
		} else if (pwf.bitspersample == 16) {
			measured_lufs = calculate_lufs16();
		} else {
			if (!quiet)
				fprintf(stderr, "Unsupported bit depth for LUFS calculation.\n");
			VirtualFree(buf, 0, MEM_RELEASE);
			pcmwav_close(&pwf);
			return 1;
		}
		
		if (!quiet) {
			fprintf(stderr, "\rMeasured loudness: %.1f LUFS\n", measured_lufs);
			fprintf(stderr, "Target loudness: %.1f LUFS\n", target_lufs);
		}
		
		// Calculate gain adjustment
		double lufs_delta = target_lufs - measured_lufs;
		ratio = pow(10.0, lufs_delta / 20.0);
		
		// Optional: Apply peak limiting to prevent clipping
		if (normpercent < 100.0) {
			double max_ratio;
			if (pwf.bitspersample == 8) {
				signed char mins, maxs;
				if (!quiet)
					fprintf(stderr, "Pass 1b: Finding peaks for limiting...\n");
				if (getpeaks8(&mins, &maxs)) {
					if (mins == -128) mins = -127;
					if ((-mins) > maxs) maxs = -mins;
					if (maxs > 0) {
						max_ratio = (127.0 * normpercent) / ((double)maxs * 100.0);
						if (ratio > max_ratio) {
							if (!quiet)
								fprintf(stderr, "Limiting gain to prevent clipping (%.1f dB reduction)\n", 
									20.0 * log10(ratio / max_ratio));
							ratio = max_ratio;
						}
					}
				}
			} else if (pwf.bitspersample == 16) {
				signed short mins, maxs;
				if (!quiet)
					fprintf(stderr, "Pass 1b: Finding peaks for limiting...\n");
				if (getpeaks16(&mins, &maxs)) {
					if (mins == -32768) mins = -32767;
					if ((-mins) > maxs) maxs = -mins;
					if (maxs > 0) {
						max_ratio = (32767.0 * normpercent) / ((double)maxs * 100.0);
						if (ratio > max_ratio) {
							if (!quiet)
								fprintf(stderr, "Limiting gain to prevent clipping (%.1f dB reduction)\n", 
									20.0 * log10(ratio / max_ratio));
							ratio = max_ratio;
						}
					}
				}
			}
		}
	}

	if (ratio == 1) {
		if (!quiet)
			fprintf(stderr, "No amplification required; skipping.\n");
		if (nooverwrite) {
			/* copy existing data */
			ndata = passthrough();
			VirtualFree(buf, 0, MEM_RELEASE);
			pcmwav_close(&pwf);
			CloseHandle(outf);
		}
		return 3;
	} else if (ratio < 1) {
		if (!quiet)
			fprintf(stderr, "Performing attenuation of %.03f dB\n", 20.0 * log10(ratio));
	} else if (ratio > 1) {
		if (!quiet)
			fprintf(stderr, "Performing amplification of %.03f dB\n", 20.0 * log10(ratio));
	}

	if (usemingain) {
		if (fabs(20.0 * log10(ratio)) < mingain) {
			if (!quiet)
				fprintf(stderr, "Level is smaller than %.03f dB, aborting.\n", mingain);
			if (nooverwrite) {
				/* copy existing data */
				ndata = passthrough();
				VirtualFree(buf, 0, MEM_RELEASE);
				pcmwav_close(&pwf);
				CloseHandle(outf);
			}
			return 3;
		}
	}

	if (prompt) {
		char	inanswer;
		fflush(stdin);
		fprintf(stderr, "\nStart normalization? (Y/N) ");
		inanswer = getchar();
		if ((inanswer != 'y') && (inanswer != 'Y')) {
			VirtualFree(buf, 0, MEM_RELEASE);
			pcmwav_close(&pwf);
			if (nooverwrite)
				CloseHandle(outf);
			return 5;
		}
	}

	if (!quiet)
		fprintf(stderr, "\nAmplifying...\n");

	sclk = clock();
	if (pwf.bitspersample == 8) {
		table8 = (signed char*)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_READWRITE);

		if (table8 == NULL) {
			if (!quiet)
				fprintf(stderr, "Cannot allocate translation table in memory.\n");
			return 4;
		}

		make_table8();
		ndata = amplify8();
		VirtualFree(table8, 0, MEM_RELEASE);

	} else if (pwf.bitspersample == 16) {
		table16 = (signed short*)VirtualAlloc(NULL, 131072, MEM_COMMIT, PAGE_READWRITE);

		if (table16 == NULL) {
			if (!quiet)
				fprintf(stderr, "Cannot allocate translation table in memory.\n");
			return 4;
		}
		make_table16();
		ndata = amplify16();
		VirtualFree(table16, 0, MEM_RELEASE);
	}
	eclk = clock();

	if (!quiet)
		fprintf(stderr, "\n\nDone.\n");

	atime = (double)(eclk - sclk) / (double)CLOCKS_PER_SEC;

	if (atime < 1.0) {
		if (!quiet)
			fprintf(stderr, "Time taken: %.01f sec.\n", atime);
	} else {
		if (!quiet)
			fprintf(stderr, "Time taken: %.01f sec. (throughput: %.03f MBps)\n",
				atime, ((double)ndata / 1048576.0) / atime);
	}

	VirtualFree(buf, 0, MEM_RELEASE);
	pcmwav_close(&pwf);

	if (nooverwrite)
		CloseHandle(outf);

	return 0;
}

#pragma optimize("", off)
void make_table8(void) {
	unsigned char	i = 0;

	do {
		if (((signed char)i * ratio) > 127.0)
			table8[i ^ 0x80] = (signed char)0xFF;
		else if (((signed char)i * ratio) < -127.0)
			table8[i ^ 0x80] = 0x00;
		else
			table8[i ^ 0x80] = (signed char)(((signed char)i) * ratio) ^ 0x80;
	} while (++i);
}
#pragma optimize("", on)

#pragma optimize("", off)
void make_table16(void) {
	unsigned short	i = 0;

	do {
		if (((signed short)i * ratio) > 32767)
			table16[i] = 32767;
		else if (((signed short)i * ratio) < -32767)
			table16[i] = -32767;
		else
			table16[i] = (signed short)(((signed short)i) * ratio);
	} while (++i);
}
#pragma optimize("", on)

int getpeaks8(signed char *minpeak, signed char *maxpeak) {
	unsigned long				i, ndone = 0, readn;
	register signed char		minp = 0, maxp = 0, cur;
	int							npercent, lastn = -1;
	unsigned long				*stats;
	unsigned long				numstat;

	if (smartpeak) {
		// allocate memory for the sample statistics
		stats = (unsigned long*)VirtualAlloc(NULL, sizeof(unsigned long) * 256, MEM_COMMIT, PAGE_READWRITE);
		
		if (stats == NULL) {
			if (!quiet)
				fprintf(stderr, "Cannot allocate buffer in memory.\n");
			return 0;
		}

		for (i = 0; i < 256; i++)
			stats[i] = 0;

		numstat = 0;
	}

	
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;

	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 0;
	}

	while (readn) {
		for (i = 0; i < readn; i++) {
			cur = buf8[i] ^ 0x80;

			if (smartpeak) {
				stats[128 + cur]++;
				numstat++;
			} else {
				if (cur < minp)
					minp = cur;
				if (cur > maxp)
					maxp = cur;	
			}
		}

		ndone += readn;

		if (!quiet) {
			npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
			if (npercent > lastn) {
				fprintf(stderr, "\r%d%%", npercent);
				fflush(stderr);
				lastn = npercent;
			}
		}

		readn = iobufsize;
		if (readn > (pwf.ndatabytes - ndone))
			readn = pwf.ndatabytes - ndone;

		if (readn) {
			if (!pcmwav_read(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		}
		else
			break;
	}

	if (smartpeak) {
		// let's find how many samples is <percent> of the max
		numstat *= 1.0 - (peakpercent / 100.0);
		// let's use this to accumulate values
		ndone = 0;
		// let's count the min sample value that has the given percentile
		for (i = 0; (i < 256) && (ndone <= numstat); i++)
			ndone += stats[i];
		minp = i - 129;
		// let's count the max sample value that has the given percentile
		ndone = 0;
		for (i = 255; (i >= 0) && (ndone <= numstat); i--)
			ndone += stats[i];
		maxp = i - 127;
		VirtualFree(stats, 0, MEM_RELEASE);
	}

	pcmwav_rewind(&pwf);

	*minpeak = minp;
	*maxpeak = maxp;

	return 1;
}

int getpeaks16(signed short *minpeak, signed short *maxpeak) {
	unsigned long				i, ndone = 0, readn;
	register signed short		minp = 0, maxp = 0, cur;
	int							npercent, lastn = -1;
	unsigned long				*stats;
	unsigned long				numstat;

	if (smartpeak) {
		// allocate memory for the sample statistics
		stats = (unsigned long*)VirtualAlloc(NULL, sizeof(unsigned long) * 65536, MEM_COMMIT, PAGE_READWRITE);
		
		if (stats == NULL) {
			if (!quiet)
				fprintf(stderr, "Cannot allocate buffer in memory.\n");
			return 0;
		}

		for (i = 0; i < 65536; i++)
			stats[i] = 0;

		numstat = 0;
	}

	
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;

	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 0;
	}

	while (readn) {
		for (i = 0; i < (readn>>1); i++) {
			cur = buf16[i];
			if (smartpeak) {
				stats[32768 + cur]++;
				numstat++;
			} else {
				if (cur < minp)
					minp = cur;
				if (cur > maxp)
					maxp = cur;
			}
		}

		ndone += readn;

		if (!quiet) {
			npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
			if (npercent > lastn) {
				fprintf(stderr, "\r%d%%", npercent);
				fflush(stderr);
				lastn = npercent;
			}
		}

		readn = iobufsize;
		if (readn > (pwf.ndatabytes - ndone))
			readn = pwf.ndatabytes - ndone;

		if (readn) {
			if (!pcmwav_read(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		}
		else
			break;
	}

	if (smartpeak) {
		// let's find how many samples is <percent> of the max
		numstat *= 1.0 - (peakpercent / 100.0);
		// let's use this to accumulate values
		ndone = 0;
		// let's count the min sample value that has the given percentile
		for (i = 0; (i < 65536) && (ndone <= numstat); i++)
			ndone += stats[i];
		minp = i - 32769;
		// let's count the max sample value that has the given percentile
		ndone = 0;
		for (i = 65535; (i >= 0) && (ndone <= numstat); i--)
			ndone += stats[i];
		maxp = i - 32767;
		VirtualFree(stats, 0, MEM_RELEASE);
	}

	pcmwav_rewind(&pwf);

	*minpeak = minp;
	*maxpeak = maxp;

	return 1;
}

unsigned long amplify8(void) {
	unsigned long	i, ndone = 0, readn;
	long			nwr;
	int				npercent, lastn = -1;
	
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;

	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 0;
	}

	while (readn) {
		for (i = 0; i < readn; i++) {
			buf8[i] = table8[(unsigned char)buf8[i]];
		}

		if (!nooverwrite) {
			nwr = -((long)readn);
			pcmwav_seek(&pwf, nwr);

			if (!pcmwav_write(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		} else {
			DWORD	nwritten;
			WriteFile(outf, buf, readn, &nwritten, NULL);
			if (nwritten != readn) {
				if (!quiet)
					fprintf(stderr, "Output file write error.\n");
				return 0;
			}
		}

		ndone += readn;

		if (!quiet) {
			npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
			if (npercent > lastn) {
				fprintf(stderr, "\r%d%%", npercent);
				fflush(stderr);
				lastn = npercent;
			}
		}

		readn = iobufsize;
		if (readn > (pwf.ndatabytes - ndone))
			readn = pwf.ndatabytes - ndone;

		if (readn) {
			if (!pcmwav_read(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		}
		else
			break;
	}

	return ndone;
}

unsigned long amplify16(void) {
	unsigned long	i, ndone = 0, readn;
	long			nwr;
	int				npercent, lastn = -1;
	
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;

	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 0;
	}

	while (readn) {
		for (i = 0; i < (readn>>1); i++) {
			buf16[i] = table16[buf16[i]];
		}

		if (!nooverwrite) {
			nwr = -((long)readn);
			pcmwav_seek(&pwf, nwr);
			
			if (!pcmwav_write(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		} else {
			DWORD nwritten;
			WriteFile(outf, buf, readn, &nwritten, NULL);
			if (nwritten != readn) {
				if (!quiet)
					fprintf(stderr, "Output file write error.\n");
				return 0;
			}
		}

		ndone += readn;

		if (!quiet) {
			npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
			if (npercent > lastn) {
				fprintf(stderr, "\r%d%%", npercent);
				fflush(stderr);
				lastn = npercent;
			}
		}

		readn = iobufsize;
		if (readn > (pwf.ndatabytes - ndone))
			readn = pwf.ndatabytes - ndone;

		if (readn) {
			if (!pcmwav_read(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		}
		else
			break;
	}

	return ndone;
}

unsigned long passthrough(void) {
	unsigned long	ndone = 0, readn;
	int				npercent, lastn = -1;
	
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;

	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		return 0;
	}

	while (readn) {
		DWORD nwritten;
		WriteFile(outf, buf, readn, &nwritten, NULL);
		if (nwritten != readn) {
			if (!quiet)
				fprintf(stderr, "Output file write error.\n");
			return 0;
		}

		ndone += readn;

		if (!quiet) {
			npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
			if (npercent > lastn) {
				fprintf(stderr, "\r%d%%", npercent);
				fflush(stderr);
				lastn = npercent;
			}
		}

		readn = iobufsize;
		if (readn > (pwf.ndatabytes - ndone))
			readn = pwf.ndatabytes - ndone;

		if (readn) {
			if (!pcmwav_read(&pwf, buf, readn)) {
				if (!quiet)
					fprintf(stderr, "%s\n", pcmwav_error);
				return 0;
			}
		}
		else
			break;
	}

	return ndone;
}

// Comparison function for qsort (for LUFS gating)
int compare_double(const void *a, const void *b) {
	double diff = (*(double*)a - *(double*)b);
	if (diff < 0) return -1;
	if (diff > 0) return 1;
	return 0;
}

// Initialize K-weighting filters according to ITU-R BS.1770-4
void init_k_weighting(k_weighting *kw, unsigned long samplerate) {
	double f0, Q, K, Vh, Vb, a0;
	double omega, cosw, sinw, alpha, beta;
	
	// Clear state
	kw->shelf.z1 = kw->shelf.z2 = 0.0;
	kw->highpass.z1 = kw->highpass.z2 = 0.0;
	
	// High-shelf filter (pre-filter stage 1)
	// Fc = 1681.974 Hz, Gain = +4.0 dB, Q = 0.7071
	f0 = 1681.974;
	Q = 0.7071;
	K = tan(3.141592653589793 * f0 / samplerate);
	Vh = pow(10.0, 4.0 / 20.0);
	Vb = pow(Vh, 0.4996667741545416);
	
	a0 = 1.0 + K / Q + K * K;
	kw->shelf.b0 = (Vh + Vb * K / Q + K * K) / a0;
	kw->shelf.b1 = 2.0 * (K * K - Vh) / a0;
	kw->shelf.b2 = (Vh - Vb * K / Q + K * K) / a0;
	kw->shelf.a1 = 2.0 * (K * K - 1.0) / a0;
	kw->shelf.a2 = (1.0 - K / Q + K * K) / a0;
	
	// High-pass filter (RLB weighting stage 2)
	// Fc = 38.13547 Hz, Q = 0.5
	f0 = 38.13547;
	Q = 0.5;
	omega = 2.0 * 3.141592653589793 * f0 / samplerate;
	cosw = cos(omega);
	sinw = sin(omega);
	alpha = sinw / (2.0 * Q);
	
	a0 = 1.0 + alpha;
	kw->highpass.b0 = (1.0 + cosw) / 2.0 / a0;
	kw->highpass.b1 = -(1.0 + cosw) / a0;
	kw->highpass.b2 = (1.0 + cosw) / 2.0 / a0;
	kw->highpass.a1 = -2.0 * cosw / a0;
	kw->highpass.a2 = (1.0 - alpha) / a0;
}

// Apply K-weighting to a single sample using biquad filters
double apply_k_weighting(k_weighting *kw, double sample) {
	double out;
	
	// Apply high-shelf filter (Direct Form II Transposed)
	out = kw->shelf.b0 * sample + kw->shelf.z1;
	kw->shelf.z1 = kw->shelf.b1 * sample - kw->shelf.a1 * out + kw->shelf.z2;
	kw->shelf.z2 = kw->shelf.b2 * sample - kw->shelf.a2 * out;
	sample = out;
	
	// Apply high-pass filter (Direct Form II Transposed)
	out = kw->highpass.b0 * sample + kw->highpass.z1;
	kw->highpass.z1 = kw->highpass.b1 * sample - kw->highpass.a1 * out + kw->highpass.z2;
	kw->highpass.z2 = kw->highpass.b2 * sample - kw->highpass.a2 * out;
	
	return out;
}

// Calculate LUFS for 8-bit audio with gating
double calculate_lufs8(void) {
	unsigned long block_samples, hop_samples, max_blocks, block_count = 0;
	unsigned long i, j, readn, ndone = 0;
	double *block_loudness;
	k_weighting kw_left, kw_right;
	int npercent, lastn = -1;
	double integrated_loudness, gated_loudness;
	unsigned long blocks_to_use, valid_blocks;
	unsigned short nchannels = pwf.nchannels;
	
	// Calculate block parameters (400ms blocks, 100ms hop) - per channel
	block_samples = (unsigned long)(pwf.samplerate * 0.4);
	hop_samples = (unsigned long)(pwf.samplerate * 0.1);
	max_blocks = (pwf.ndatabytes / (hop_samples * nchannels)) + 1;
	
	// Allocate memory for block loudness values
	block_loudness = (double*)VirtualAlloc(NULL, sizeof(double) * max_blocks, MEM_COMMIT, PAGE_READWRITE);
	if (block_loudness == NULL) {
		if (!quiet)
			fprintf(stderr, "Cannot allocate memory for LUFS calculation.\n");
		return -70.0;
	}
	
	// Initialize K-weighting filters (one per channel)
	init_k_weighting(&kw_left, pwf.samplerate);
	if (nchannels == 2)
		init_k_weighting(&kw_right, pwf.samplerate);
	
	// Allocate sliding window buffers (one per channel)
	double *window_left = (double*)VirtualAlloc(NULL, sizeof(double) * block_samples, MEM_COMMIT, PAGE_READWRITE);
	double *window_right = (nchannels == 2) ? 
		(double*)VirtualAlloc(NULL, sizeof(double) * block_samples, MEM_COMMIT, PAGE_READWRITE) : NULL;
	
	if (window_left == NULL || (nchannels == 2 && window_right == NULL)) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		if (window_left) VirtualFree(window_left, 0, MEM_RELEASE);
		if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
		if (!quiet)
			fprintf(stderr, "Cannot allocate memory for LUFS calculation.\n");
		return -70.0;
	}
	
	unsigned long window_pos = 0, samples_in_window = 0;
	
	// Read initial buffer
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;
	
	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		VirtualFree(window_left, 0, MEM_RELEASE);
		if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
		return -70.0;
	}
	
	unsigned long buf_pos = 0;
	
	while (readn > 0 || samples_in_window >= block_samples) {
		// Fill window with samples (interleaved for stereo)
		while (buf_pos < readn && samples_in_window < block_samples) {
			// Left channel (or mono)
			signed char sample_left = buf8[buf_pos++] ^ 0x80;
			double norm_left = sample_left / 128.0;
			window_left[window_pos] = apply_k_weighting(&kw_left, norm_left);
			
			// Right channel (if stereo)
			if (nchannels == 2) {
				signed char sample_right = buf8[buf_pos++] ^ 0x80;
				double norm_right = sample_right / 128.0;
				window_right[window_pos] = apply_k_weighting(&kw_right, norm_right);
			}
			
			window_pos++;
			samples_in_window++;
			if (window_pos >= block_samples) window_pos = 0;
		}
		
		// Process complete block
		if (samples_in_window >= block_samples) {
			double sum_squares_left = 0.0, sum_squares_right = 0.0;
			
			// Calculate mean square per channel
			for (i = 0; i < block_samples; i++) {
				unsigned long idx = (window_pos + i) % block_samples;
				sum_squares_left += window_left[idx] * window_left[idx];
				if (nchannels == 2)
					sum_squares_right += window_right[idx] * window_right[idx];
			}
			
			// Average across channels (ITU-R BS.1770-4)
			double mean_square;
			if (nchannels == 2) {
				mean_square = (sum_squares_left + sum_squares_right) / (2.0 * block_samples);
			} else {
				mean_square = sum_squares_left / block_samples;
			}
			
			if (mean_square > 0.0) {
				block_loudness[block_count++] = -0.691 + 10.0 * log10(mean_square);
			} else {
				block_loudness[block_count++] = -70.0;
			}
			
			// Slide window by hop_samples
			unsigned long to_skip = hop_samples;
			while (to_skip > 0 && samples_in_window > 0) {
				window_pos++;
				if (window_pos >= block_samples) window_pos = 0;
				samples_in_window--;
				to_skip--;
			}
		}
		
		// Read more data if needed
		if (buf_pos >= readn) {
			ndone += readn;
			
			if (!quiet) {
				npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
				if (npercent > lastn) {
					fprintf(stderr, "\rPass 1 (LUFS): %d%%", npercent);
					fflush(stderr);
					lastn = npercent;
				}
			}
			
			readn = iobufsize;
			if (readn > (pwf.ndatabytes - ndone))
				readn = pwf.ndatabytes - ndone;
			
			if (readn > 0) {
				if (!pcmwav_read(&pwf, buf, readn)) {
					if (!quiet)
						fprintf(stderr, "%s\n", pcmwav_error);
					VirtualFree(block_loudness, 0, MEM_RELEASE);
					VirtualFree(window_left, 0, MEM_RELEASE);
					if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
					return -70.0;
				}
				buf_pos = 0;
			}
		}
		
		if (readn == 0 && samples_in_window < block_samples)
			break;
	}
	
	VirtualFree(window_left, 0, MEM_RELEASE);
	if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
	
	if (block_count == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return -70.0;
	}
	
	// Apply percentile gating if specified
	if (gate_percentile < 100.0) {
		qsort(block_loudness, block_count, sizeof(double), compare_double);
		blocks_to_use = (unsigned long)(block_count * gate_percentile / 100.0);
		if (blocks_to_use < 1) blocks_to_use = 1;
		block_count = blocks_to_use;
	}
	
	// Apply absolute gate (-70 LUFS)
	double sum_loudness = 0.0;
	valid_blocks = 0;
	for (i = 0; i < block_count; i++) {
		if (block_loudness[i] > -70.0) {
			sum_loudness += pow(10.0, block_loudness[i] / 10.0);
			valid_blocks++;
		}
	}
	
	if (valid_blocks == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return -70.0;
	}
	
	double avg_loudness = -0.691 + 10.0 * log10(sum_loudness / valid_blocks);
	
	// Apply relative gate (-10 LU below average)
	double relative_threshold = avg_loudness - 10.0;
	sum_loudness = 0.0;
	valid_blocks = 0;
	
	for (i = 0; i < block_count; i++) {
		if (block_loudness[i] >= relative_threshold) {
			sum_loudness += pow(10.0, block_loudness[i] / 10.0);
			valid_blocks++;
		}
	}
	
	if (valid_blocks == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return avg_loudness;
	}
	
	integrated_loudness = -0.691 + 10.0 * log10(sum_loudness / valid_blocks);
	
	VirtualFree(block_loudness, 0, MEM_RELEASE);
	pcmwav_rewind(&pwf);
	
	return integrated_loudness;
}

// Calculate LUFS for 16-bit audio with gating
double calculate_lufs16(void) {
	unsigned long block_samples, hop_samples, max_blocks, block_count = 0;
	unsigned long i, j, readn, ndone = 0;
	double *block_loudness;
	k_weighting kw_left, kw_right;
	int npercent, lastn = -1;
	double integrated_loudness, gated_loudness;
	unsigned long blocks_to_use, valid_blocks;
	unsigned short nchannels = pwf.nchannels;
	
	// Calculate block parameters (400ms blocks, 100ms hop) - per channel
	block_samples = (unsigned long)(pwf.samplerate * 0.4);
	hop_samples = (unsigned long)(pwf.samplerate * 0.1);
	max_blocks = (pwf.ndatabytes / (hop_samples * 2 * nchannels)) + 1;
	
	// Allocate memory for block loudness values
	block_loudness = (double*)VirtualAlloc(NULL, sizeof(double) * max_blocks, MEM_COMMIT, PAGE_READWRITE);
	if (block_loudness == NULL) {
		if (!quiet)
			fprintf(stderr, "Cannot allocate memory for LUFS calculation.\n");
		return -70.0;
	}
	
	// Initialize K-weighting filters (one per channel)
	init_k_weighting(&kw_left, pwf.samplerate);
	if (nchannels == 2)
		init_k_weighting(&kw_right, pwf.samplerate);
	
	// Allocate sliding window buffers (one per channel)
	double *window_left = (double*)VirtualAlloc(NULL, sizeof(double) * block_samples, MEM_COMMIT, PAGE_READWRITE);
	double *window_right = (nchannels == 2) ? 
		(double*)VirtualAlloc(NULL, sizeof(double) * block_samples, MEM_COMMIT, PAGE_READWRITE) : NULL;
	
	if (window_left == NULL || (nchannels == 2 && window_right == NULL)) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		if (window_left) VirtualFree(window_left, 0, MEM_RELEASE);
		if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
		if (!quiet)
			fprintf(stderr, "Cannot allocate memory for LUFS calculation.\n");
		return -70.0;
	}
	
	unsigned long window_pos = 0, samples_in_window = 0;
	
	// Read initial buffer
	readn = iobufsize;
	if (pwf.ndatabytes < iobufsize)
		readn = pwf.ndatabytes;
	
	if (!pcmwav_read(&pwf, buf, readn)) {
		if (!quiet)
			fprintf(stderr, "%s\n", pcmwav_error);
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		VirtualFree(window_left, 0, MEM_RELEASE);
		if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
		return -70.0;
	}
	
	unsigned long buf_pos = 0;
	
	while (readn > 0 || samples_in_window >= block_samples) {
		// Fill window with samples (interleaved for stereo)
		while (buf_pos < (readn >> 1) && samples_in_window < block_samples) {
			// Left channel (or mono)
			signed short sample_left = buf16[buf_pos++];
			double norm_left = sample_left / 32768.0;
			window_left[window_pos] = apply_k_weighting(&kw_left, norm_left);
			
			// Right channel (if stereo)
			if (nchannels == 2) {
				signed short sample_right = buf16[buf_pos++];
				double norm_right = sample_right / 32768.0;
				window_right[window_pos] = apply_k_weighting(&kw_right, norm_right);
			}
			
			window_pos++;
			samples_in_window++;
			if (window_pos >= block_samples) window_pos = 0;
		}
		
		// Process complete block
		if (samples_in_window >= block_samples) {
			double sum_squares_left = 0.0, sum_squares_right = 0.0;
			
			// Calculate mean square per channel
			for (i = 0; i < block_samples; i++) {
				unsigned long idx = (window_pos + i) % block_samples;
				sum_squares_left += window_left[idx] * window_left[idx];
				if (nchannels == 2)
					sum_squares_right += window_right[idx] * window_right[idx];
			}
			
			// Average across channels (ITU-R BS.1770-4)
			double mean_square;
			if (nchannels == 2) {
				mean_square = (sum_squares_left + sum_squares_right) / (2.0 * block_samples);
			} else {
				mean_square = sum_squares_left / block_samples;
			}
			
			if (mean_square > 0.0) {
				block_loudness[block_count++] = -0.691 + 10.0 * log10(mean_square);
			} else {
				block_loudness[block_count++] = -70.0;
			}
			
			// Slide window by hop_samples
			unsigned long to_skip = hop_samples;
			while (to_skip > 0 && samples_in_window > 0) {
				window_pos++;
				if (window_pos >= block_samples) window_pos = 0;
				samples_in_window--;
				to_skip--;
			}
		}
		
		// Read more data if needed
		if (buf_pos >= (readn >> 1)) {
			ndone += readn;
			
			if (!quiet) {
				npercent = (int)(100.0 * ((double)ndone / (double)pwf.ndatabytes));
				if (npercent > lastn) {
					fprintf(stderr, "\rPass 1 (LUFS): %d%%", npercent);
					fflush(stderr);
					lastn = npercent;
				}
			}
			
			readn = iobufsize;
			if (readn > (pwf.ndatabytes - ndone))
				readn = pwf.ndatabytes - ndone;
			
			if (readn > 0) {
				if (!pcmwav_read(&pwf, buf, readn)) {
					if (!quiet)
						fprintf(stderr, "%s\n", pcmwav_error);
					VirtualFree(block_loudness, 0, MEM_RELEASE);
					VirtualFree(window_left, 0, MEM_RELEASE);
					if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
					return -70.0;
				}
				buf_pos = 0;
			}
		}
		
		if (readn == 0 && samples_in_window < block_samples)
			break;
	}
	
	VirtualFree(window_left, 0, MEM_RELEASE);
	if (window_right) VirtualFree(window_right, 0, MEM_RELEASE);
	
	if (block_count == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return -70.0;
	}
	
	// Apply percentile gating if specified
	if (gate_percentile < 100.0) {
		qsort(block_loudness, block_count, sizeof(double), compare_double);
		blocks_to_use = (unsigned long)(block_count * gate_percentile / 100.0);
		if (blocks_to_use < 1) blocks_to_use = 1;
		block_count = blocks_to_use;
	}
	
	// Apply absolute gate (-70 LUFS)
	double sum_loudness = 0.0;
	valid_blocks = 0;
	for (i = 0; i < block_count; i++) {
		if (block_loudness[i] > -70.0) {
			sum_loudness += pow(10.0, block_loudness[i] / 10.0);
			valid_blocks++;
		}
	}
	
	if (valid_blocks == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return -70.0;
	}
	
	double avg_loudness = -0.691 + 10.0 * log10(sum_loudness / valid_blocks);
	
	// Apply relative gate (-10 LU below average)
	double relative_threshold = avg_loudness - 10.0;
	sum_loudness = 0.0;
	valid_blocks = 0;
	
	for (i = 0; i < block_count; i++) {
		if (block_loudness[i] >= relative_threshold) {
			sum_loudness += pow(10.0, block_loudness[i] / 10.0);
			valid_blocks++;
		}
	}
	
	if (valid_blocks == 0) {
		VirtualFree(block_loudness, 0, MEM_RELEASE);
		return avg_loudness;
	}
	
	integrated_loudness = -0.691 + 10.0 * log10(sum_loudness / valid_blocks);
	
	VirtualFree(block_loudness, 0, MEM_RELEASE);
	pcmwav_rewind(&pwf);
	
	return integrated_loudness;
}

// Check if file is completely written and ready to process
int is_file_ready(char *filepath) {
	HANDLE hFile;
	DWORD waited = 0;
	
	// Try to open the file with exclusive access
	// If it fails, the file is still being written
	while (waited < 5000) {  // Wait up to 5 seconds
		hFile = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			Sleep(500);  // Extra delay to ensure file is complete
			return 1;
		}
		
		// Check if file exists but is locked
		if (GetLastError() == ERROR_SHARING_VIOLATION) {
			Sleep(100);
			waited += 100;
			continue;
		}
		
		// File doesn't exist or other error
		return 0;
	}
	
	return 0;  // Timeout
}

// Move processed file to output folder
int move_file_to_output(char *srcpath, char *outfolder) {
	char destpath[_MAX_PATH];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char final_dest[_MAX_PATH];
	int counter = 1;
	
	// Extract filename from source path
	_splitpath(srcpath, NULL, NULL, fname, ext);
	
	// Build destination path
	sprintf(destpath, "%s\\%s%s", outfolder, fname, ext);
	
	// Handle name conflicts
	strcpy(final_dest, destpath);
	while (GetFileAttributes(final_dest) != INVALID_FILE_ATTRIBUTES) {
		sprintf(final_dest, "%s\\%s_%d%s", outfolder, fname, counter++, ext);
		if (counter > 9999) {
			fprintf(stderr, "Error: Too many files with same name in output folder.\n");
			return 0;
		}
	}
	
	// Move the file
	if (MoveFile(srcpath, final_dest)) {
		return 1;
	} else {
		fprintf(stderr, "Error moving file to output folder: %s\n", final_dest);
		return 0;
	}
}

// Process all existing WAV files in the folder
void process_existing_files(char *folder, char *outfolder) {
	char search_path[_MAX_PATH];
	char fullpath[_MAX_PATH];
	char filename[_MAX_PATH];
	struct _finddata_t fileinfo;
	intptr_t hFind;
	int result;
	
	// Search for all .wav files
	sprintf(search_path, "%s\\*.wav", folder);
	
	hFind = _findfirst(search_path, &fileinfo);
	if (hFind == -1) {
		// No files found, that's okay
		return;
	}
	
	do {
		strcpy(filename, fileinfo.name);
		sprintf(fullpath, "%s\\%s", folder, filename);
		
		if (!quiet)
			fprintf(stderr, "Found existing file: %s\n", filename);
		
		// Check if file is ready
		if (is_file_ready(fullpath)) {
			if (!quiet)
				fprintf(stderr, "Processing: %s\n", filename);
			
			// Process the file
			result = process_file(fullpath);
			
			if (result == 0 || result == 3) {
				// Success or no amplification needed
				if (move_file_to_output(fullpath, outfolder)) {
					if (!quiet)
						fprintf(stderr, "Moved to output folder: %s\n\n", filename);
				} else {
					if (!quiet)
						fprintf(stderr, "Warning: Could not move file to output folder.\n\n");
				}
			} else {
				if (!quiet)
					fprintf(stderr, "Error processing file, leaving in watch folder.\n\n");
			}
		} else {
			if (!quiet)
				fprintf(stderr, "Warning: File not ready or locked, skipping: %s\n\n", filename);
		}
		
	} while (_findnext(hFind, &fileinfo) == 0);
	
	_findclose(hFind);
}

// Watch folder for new WAV files and process them
int watch_folder_mode(char *folder, char *outfolder) {
	HANDLE hDir;
	char buffer[4096];
	DWORD bytesReturned;
	FILE_NOTIFY_INFORMATION *fni;
	char fullpath[_MAX_PATH];
	char filename[_MAX_PATH];
	WCHAR *wfilename;
	int len;
	
	// Create output folder if it doesn't exist
	CreateDirectory(outfolder, NULL);
	
	// Open the directory for monitoring
	hDir = CreateFile(folder,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	
	if (hDir == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: Cannot open watch folder: %s\n", folder);
		fprintf(stderr, "Make sure the folder exists and you have permission to access it.\n");
		return 1;
	}
	
	if (!quiet)
		fprintf(stderr, "Watching for new .wav files...\n\n");
	
	// Process any existing files first
	if (!quiet)
		fprintf(stderr, "Checking for existing files in folder...\n");
	process_existing_files(folder, outfolder);
	
	// Main watch loop
	while (1) {
		// Wait for changes
		if (ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), FALSE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
			&bytesReturned, NULL, NULL)) {
			
			fni = (FILE_NOTIFY_INFORMATION*)buffer;
			
			do {
				// Convert wide char filename to regular char first
				wfilename = fni->FileName;
				len = WideCharToMultiByte(CP_ACP, 0, wfilename, fni->FileNameLength / sizeof(WCHAR),
					filename, sizeof(filename), NULL, NULL);
				filename[len] = '\0';
				
				// Only process file creation and modification
				if (fni->Action == FILE_ACTION_ADDED || fni->Action == FILE_ACTION_MODIFIED) {
					// Check if it's a WAV file (must be at least 5 chars: "a.wav")
					if (strlen(filename) > 4 && _stricmp(filename + strlen(filename) - 4, ".wav") == 0) {
						// Build full path
						sprintf(fullpath, "%s\\%s", folder, filename);
						
						// Check if file still exists (might have been processed already)
						DWORD attrs = GetFileAttributes(fullpath);
						if (attrs == INVALID_FILE_ATTRIBUTES) {
							// File doesn't exist, skip (probably already processed)
							goto next_notification;
						}
						
						// Wait for file to be ready
						if (!quiet)
							fprintf(stderr, "New file detected: %s\n", filename);
						
						if (is_file_ready(fullpath)) {
							// Double-check file still exists before processing
							attrs = GetFileAttributes(fullpath);
							if (attrs == INVALID_FILE_ATTRIBUTES) {
								if (!quiet)
									fprintf(stderr, "File disappeared before processing, skipping.\n\n");
								goto next_notification;
							}
							
							if (!quiet)
								fprintf(stderr, "Processing: %s\n", filename);
							
							// Process the file
							int result = process_file(fullpath);
							
							if (result == 0 || result == 3) {
								// Success or no amplification needed
								if (move_file_to_output(fullpath, outfolder)) {
									if (!quiet)
										fprintf(stderr, "Moved to output folder: %s\n\n", filename);
								} else {
									if (!quiet)
										fprintf(stderr, "Warning: Could not move file to output folder.\n\n");
								}
							} else {
								if (!quiet)
									fprintf(stderr, "Error processing file, leaving in watch folder.\n\n");
							}
						} else {
							if (!quiet)
								fprintf(stderr, "Warning: File not ready or locked, skipping: %s\n\n", filename);
						}
					}
				}
				
next_notification:
				// Move to next notification
				if (fni->NextEntryOffset == 0)
					break;
				fni = (FILE_NOTIFY_INFORMATION*)((char*)fni + fni->NextEntryOffset);
				
			} while (1);
			
			// After processing all events, check for any remaining files
			// This catches files that may have been missed by the event notification
			process_existing_files(folder, outfolder);
			
		} else {
			fprintf(stderr, "Error monitoring directory. Aborting.\n");
			CloseHandle(hDir);
			return 1;
		}
	}
	
	CloseHandle(hDir);
	return 0;
}

void usage(void) {
	fprintf(stderr, "\n%s\nVisit http://neon1.net/ for updates.\n\n", COPYRIGHT_NOTICE);	
	fprintf(stderr,
		"    Usage:  normalize [flags] input-file\n\n"

		"        -l <ratio>   don't find peaks but multiply each sample by <ratio>\n"
		"        -a <level>   don't find peaks; amplify by <level> (given in dB)\n"
		"        -L <lufs>    normalize to target LUFS loudness (e.g. -14 for Spotify)\n"
		"        -g <percent> gate percentile for LUFS: ignore loudest blocks (50-100%%)\n"
		"        -m <percent> normalize to <percent> % (default 100)\n"
		"        -s <percent> smartpeak: count as a peak only a signal that has the\n"
		"                     given percentile (50%%-100%%)\n"
		"        -x <level>   abort if gain increase is smaller than <level> (in dB)\n"
		"        -p           prompt before starting normalization\n"
		"        -b <size>    specify I/O buffer size (in KB; 16..16384; default 64)\n"
		"        -o <file>    write output to <file> (instead of overwriting original)\n"
		"        -w <folder>  watch mode: monitor folder for new WAV files\n"
		"        -O <folder>  output folder for watch mode (required with -w)\n"
		"        -q           quiet (no screen output)\n"
		"        -d           don't abort batch if user skips normalization of one file\n"
		"        -h           display this help\n\n"

		"    error levels: 0 = no error, 1 = I/O error, 2 = parameter error,\n"
		"                  3 = no amplification required, 4 = out of memory,\n"
		"                  5 = user abort\n\n"
		
		"    LUFS normalization examples:\n"
		"        normalize -L -14 *.wav          # Spotify standard\n"
		"        normalize -L -23 *.wav          # Broadcast standard (EBU R128)\n"
		"        normalize -L -16 -g 95 *.wav    # Ignore loudest 5%% of blocks\n"
		"        normalize -L -14 -m 98 *.wav    # LUFS with peak limiting\n\n"
		
		"    Watch mode examples:\n"
		"        normalize -L -14 -m 99 -w C:\\incoming -O C:\\processed\n"
		"        normalize -m 95 -w \"Drop Files Here\" -O \"Done\"\n"
		"        normalize -L -16 -g 95 -w input -O output -q\n\n"
		
		"	- wildcards are allowed in 'input-file' (e.g. normalize *.wav)\n"
		"	- 'input-file' needs to be a PCM WAV file.\n"
		"	- watch mode runs continuously until stopped with Ctrl+C\n");
}
