#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "../lib.h"

int fuzzle_file(source, target)
char *source, *target;
{
    FILE *source_fp, *target_fp;
    char line[256];

    if (!(source_fp = fopen(source, "r"))) 
    {
        fprintf(stderr, "! Error opening source file %s", source);
        return 1;
    }

    if (!(target_fp = fopen(target, "w"))) 
    {
        fprintf(stderr, "! Error opening target file %s\n", target);
        fclose(source_fp);
        return 1;
    }

    while (fgets(line, 256, source_fp))
    {
        fuzzle(line);
        fputs(line, target_fp);
    }

    fclose(source_fp);
    fclose(target_fp);
    return 0;
}

int check_create_dir(dir)
char *dir;
{
    DIR *d;
    struct stat st;

    if (stat(dir, &st) && errno == ENOENT) 
    {
        printf("Creating directory %s\n", dir);
        if (mkdir(dir, 0755)) 
        {
            fprintf(stderr, "! Error creating directory %s", dir);
            return 1;
        }
    }

    if (!(d = opendir(dir))) 
    {
        fprintf(stderr, "! Error opening directory %s", dir);
        return 1;
    }

    closedir(d);
    return 0;
}

int process_lang_dir(source_dir, target_dir)
char *source_dir, *target_dir;
{
    DIR *d;
    struct direct *entry;
    struct stat st;
    char source_path[64], target_path[64];
    int str_length;

    if (!(d = opendir(source_dir))) 
    {
        fprintf(stderr, "! Error opening language directory %s. Skipping\n", source_dir);
        return 0;
    }

    if (check_create_dir(target_dir))
    {
        closedir(d);
        return 1;
    }

    while (entry = readdir(d))
    {
        sprintf(source_path, "%s/%s", source_dir, entry->d_name);

        if (stat(source_path, &st) || !S_ISREG(st.st_mode))
            continue;   /* Can't stat file, or not a file */

        str_length = strlen(entry->d_name);
        if (str_length < 4 || strcmp(entry->d_name + str_length - 4, ".txt"))
            continue;   /* Only process .txt files */

        sprintf(target_path, "%s/%s", target_dir, entry->d_name);
        target_path[strlen(target_path) - 4] = 0;   /* Remove .txt extension */

        printf("Processing file %s, saving to %s\n", source_path, target_path);

        if (fuzzle_file(source_path, target_path)) 
        {
            closedir(d);
            return 1;
        }
    }

    closedir(d);
    return 0;
}

int main()
{
    char *source = "texts", *target = "data";

    char source_path[64], target_path[64];
    DIR *d;
    struct direct *entry;
    struct stat st;

    if (!(d = opendir(source)))
    {
        fprintf(stderr, "! Error opening source directory %s\n. Aborting", source);
        return 1;
    }

    if (check_create_dir(target))
    {
        fprintf(stderr, ". Aborting\n");
        closedir(d);
        return 1;
    }

    while (entry = readdir(d))
    {
        sprintf(source_path, "%s/%s", source, entry->d_name);

        if (stat(source_path, &st) || !S_ISDIR(st.st_mode) 
            || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;   /* Not a directory, or . or .. */
        }

        sprintf(target_path, "%s/%s", target, entry->d_name);

        printf("Processing language directory %s...\n", source_path);
        if (process_lang_dir(source_path, target_path)) 
        {
            fprintf(stderr, ". Aborting\n");
            closedir(d);
            return 1;
        }
    }

    closedir(d);
    return 0;
}