


#include <stdio.h>
#include <stdlib.h>


int main(int arguments_count, char **arguments_values){
    FILE *file = fopen("w:/ced/code/ced_page.html", "wb");
    if(file){
        fprintf(file, "%s", 
                "<!-- This is my first html documentation. -->\n"
                "<!DOCTYPE html>\n"
                "\n"
                "<html>\n"
                "<head>\n"
                "<style>\n"
                "h1{\n"
                "text-align: center;\n"
                "color: rgb(185, 150, 105);\n"
                "font-family: \"Consolas\", \"Courier New\", monospace;\n"
                "}\n"
                "h3{\n"
                "text-align: center;\n"
                "color: rgb(0, 205, 80);\n"
                "font-family: \"Consolas\", \"Courier New\", monospace;\n"
                "}\n"
                "a:link{\n"
                "text-decoration: none;\n"
                "}\n"
                "a:hover{\n"
                "color : red;\n"
                "text-decoration: none;\n"
                "}\n"
                "</style>\n"
                "</head>\n"
                "</style>\n"
                "<body style=\"background: rgb(28, 28, 28);\">\n"
                "<h1> Ced </h1>\n"
                "<h3>\n"
                "Ced is a programm intended to increase your typing speed. Totaly written in C, using OpenGL as a renderer. <br>\n"
                "The platform layer written in Win32 to achieve high performance. <br>\n"
                "It is still in development. So for bug reports I suggest you to write on this email address: <br>\n"
                "<h3 style=\"color: #c1ffc1;\"> Alexey.Yevtushenko.Work@gmail.com </h3>\n"
                "</h3>\n"
                "</body>\n"
                "</html>\n");

        fclose(file);
    }
    return(0);
}
