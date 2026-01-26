#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
        if (argc != 3)
        {
                fprintf(stderr, "Usage: %s input_file output_h\n", argv[0]);
                return 1;
        }

        const char *input = argv[1];
        const char *output = argv[2];

        FILE *fin = fopen(input, "rb"); // бинарный режим
        if (!fin)
        {
                perror("fopen input");
                return 1;
        }

        FILE *fout = fopen(output, "w");
        if (!fout)
        {
                perror("fopen output");
                fclose(fin);
                return 1;
        }

        // имя переменной из имени файла
        const char *var_name = strrchr(input, '/');
        if (!var_name)
                var_name = input;
        else
                var_name++;

        // заменяем . или - на _
        char name_clean[256];
        strncpy(name_clean, var_name, sizeof(name_clean));
        name_clean[sizeof(name_clean) - 1] = 0;
        for (char *p = name_clean; *p; p++)
                if (*p == '.' || *p == '-')
                        *p = '_';

        fprintf(fout, "#ifndef %s_H\n#define %s_H\n\n", name_clean, name_clean);
        fprintf(fout, "static const unsigned char %s[] = {", name_clean);

        int c;
        int count = 0;
        while ((c = fgetc(fin)) != EOF)
        {
                if (count % 12 == 0)
                        fprintf(fout, "\n    ");
                fprintf(fout, "0x%02X,", (unsigned char)c);
                count++;
        }

        fprintf(fout, "\n};\n\n");
        fprintf(fout, "static const unsigned int %s_size = %d;\n\n", name_clean, count);
        fprintf(fout, "#endif // %s_H\n", name_clean);

        fclose(fin);
        fclose(fout);

        return 0;
}