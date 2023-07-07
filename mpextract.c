#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

void help(char *name)
{
    puts("Motion Photo Extractor");
    printf("Usage: %s [OPTIONS] [INPUT_FILE]\n", name);
    puts(" -h - Show usage");
    puts(" -v - Enable verbose output");
    puts(" -i [FILENAME] - Output image part to file");
    puts(" -m [FILENAME] - Output video part to file");
}

int mpextract(char *image_in, char *image_out, char *video_out, int verbose)
{
    const unsigned char marker[] = {0x4D, 0x6F, 0x74, 0x69, 0x6F, 0x6E, 0x50, 0x68, 0x6F, 0x74, 0x6F, 0x5F, 0x44, 0x61, 0x74, 0x61};
    int marker_pos = 0;
    int marker_sz = sizeof(marker);
    size_t file_sz;
    int input_fd;
    int output_fd;
    char *file_map;
    size_t pos = 0;
    size_t cut_pos = 0;

    input_fd = open(image_in, O_RDONLY);

    if (input_fd < 0) {
        fputs("Failed to open input file!\n", stderr);
        return 1;
    }

    file_sz = lseek(input_fd, 0, SEEK_END);

    file_map = mmap(0, file_sz, PROT_READ, MAP_PRIVATE, input_fd, 0);

    if (file_map == MAP_FAILED) {
        fputs("Failed to mmap() input file!\n", stderr);
        return 1;
    }

    while (pos < (file_sz - marker_sz)) {
        while (file_map[pos] == marker[marker_pos]) {
            pos++;
            marker_pos++;

            if (marker_pos == marker_sz-1) {
                cut_pos = pos-marker_sz;
                if (verbose == 1) printf("Found marker at %d\n", cut_pos);
                goto loop_end;
            }
        }

        pos++;
        marker_pos = 0;
    }

    loop_end:
    if (cut_pos != 0) {
        if (image_out != NULL) {
            if (verbose == 1) printf("Writing out image file: %s\n", image_out);
            output_fd = open(image_out, O_CREAT|O_RDWR|O_EXCL, 0666);

            if (output_fd < 0) {
                fputs("Failed to create image output file!\n", stderr);
                return 1;
            }

            write(output_fd, file_map, cut_pos+1);
            close(output_fd);
        }

        if (video_out != NULL) {
            if (verbose == 1) printf("Writing out video file: %s\n", video_out);
            output_fd = open(video_out, O_CREAT|O_RDWR|O_EXCL, 0666);

            if (output_fd < 0) {
                fputs("Failed to create video output file!\n", stderr);
                return 1;
            }

            write(output_fd, &file_map[cut_pos+marker_sz+1], file_sz-(cut_pos+marker_sz+1));
            close(output_fd);
        }
    }

    close(input_fd);

    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int verbose = 0;
    char *image_in = NULL;
    char *image_out = NULL;
    char *video_out = NULL;

    while ((opt = getopt(argc, argv, "hvi:m:")) != -1) {
        switch (opt) {
            case 'h':
                help(argv[0]);
                return 0;

            case 'v':
                verbose = 1;
                break;

            case 'i':
                image_out = optarg;
                break;

            case 'm':
                video_out = optarg;
                break;
        }
    }

    if (optind > (argc-1)) {
        fputs("Input file not specified!\n", stderr);
        return 1;
    }

    image_in = argv[optind];

    return mpextract(image_in, image_out, video_out, verbose);
}

