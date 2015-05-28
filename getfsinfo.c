/*
 * Copyright (C) 2015 Jan Synáček
 *
 * Author: Jan Synáček <jan.synacek@gmail.com>
 * URL: https://github.com/jsynacek/getfsinfo
 * Created: May 2015
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 */

/* TODO:
 *  - add switch to display values in bytes, not only human readable
 *  - add pretty output
 */

#include <sys/statvfs.h>
#include <errno.h>
#include <mntent.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static void usage(const char *progname)
{
	die("usage: %s <file> [<file> ...]", progname);
}

static char *human_readable(unsigned long size)
{
	const char *suffixes[] = {"B", "K", "M", "G", "T"};
	const int len = sizeof suffixes / sizeof suffixes[0];
	double s = size;
	int factor = 1;
	char *out;

	while (s >= 1024.0 && factor < len) {
		s /= 1024.0;
		factor++;
	}

	if (asprintf(&out, "%.2lf%s", s, suffixes[factor - 1]) < 0)
		die("Memory allocation failed");

	return out;
}

static struct mntent copymntent(const struct mntent *mnt)
{
	struct mntent out;

	out.mnt_fsname = strdup(mnt->mnt_fsname);
	out.mnt_dir = strdup(mnt->mnt_dir);
	out.mnt_type = strdup(mnt->mnt_type);
	out.mnt_opts = strdup(mnt->mnt_opts);

	return out;
}

static struct mntent find_mntent(const char *file)
{
	FILE *fp;
	struct mntent *mnt, out;
	char *p;
	unsigned l, len = 0;

	fp = fopen("/proc/mounts", "r");
	if (!fp)
		die("Cannot open '/proc/mounts': %s", strerror(errno));

	while ((mnt = getmntent(fp))) {
		p = strstr(file, mnt->mnt_dir);
		l = strlen(mnt->mnt_dir);
		if (p == file && len < l) {
			len = l;
			out = copymntent(mnt);
		}
	}

	fclose(fp);
	return out;
}

int main(int argc, char *argv[])
{
	struct statvfs ss;
	struct mntent mnt;
	int i = 1;

	if (argc < 2)
		usage(argv[0]);

	while (i < argc) {
		if (statvfs(argv[i], &ss) < 0)
			die("Error opening '%s': %s", argv[i], strerror(errno));

		mnt = find_mntent(argv[i]);
		printf("filesystem of '%s':\n"
		       "  ID: %lx\n"
		       "  device: %s (%s)\n"
		       "  mount point: %s\n"
		       "  mount options: %s\n"
		       "  max filename length: %ld\n"
		       "  block size: %ld\n"
		       "  size: %s\n"
		       "  free: %s\n"
		       "  available: %s\n"
		       "  number of files: %ld\n",
		       argv[i],
		       ss.f_fsid,
		       mnt.mnt_fsname, mnt.mnt_type,
		       mnt.mnt_dir,
		       mnt.mnt_opts,
		       ss.f_namemax,
		       ss.f_bsize,
		       human_readable(ss.f_blocks * ss.f_bsize),
		       human_readable(ss.f_bfree * ss.f_bsize),
		       human_readable(ss.f_bavail * ss.f_bsize),
		       ss.f_files);

		i++;
	}

	return EXIT_SUCCESS;
}
