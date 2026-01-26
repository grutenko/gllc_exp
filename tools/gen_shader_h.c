#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
        if (argc != 3)
        {
                fprintf(stderr, "Usage: %s input_shader output_h\n", argv[0]);
                return 1;
        }

        FILE *fin = fopen(argv[1], "r");
        if (!fin)
        {
                perror("fopen");
                return 1;
        }

        FILE *fout = fopen(argv[2], "w");
        if (!fout)
        {
                perror("fopen");
                fclose(fin);
                return 1;
        }

        // Имя переменной
        const char *var_name = strrchr(argv[1], '/');
        if (!var_name)
                var_name = argv[1];
        else
                var_name++;
        for (char *p = (char *)var_name; *p; p++)
                if (*p == '.' || *p == '-')
                        *p = '_';

        fprintf(fout, "#ifndef %s_H\n#define %s_H\n\n", var_name, var_name);
        fprintf(fout, "static const char* %s = \n", var_name);

        char line[1024];
        while (fgets(line, sizeof(line), fin))
        {
                // Убираем \n
                size_t len = strlen(line);
                if (len > 0 && line[len - 1] == '\n')
                        line[len - 1] = 0;
                fprintf(fout, "\"%s\\n\"\n", line);
        }

        fprintf(fout, ";\n\n#endif // %s_H\n", var_name);

        fclose(fin);
        fclose(fout);
        return 0;
}
