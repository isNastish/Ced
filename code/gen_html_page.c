


#include <stdio.h>
#include <stdlib.h>


int main(int arguments_count, char **arguments_values){
    FILE *file = fopen("w:/ced/code/ced_page.html", "wb");
    if(file){
        fprintf(file, "%s", 
                "<!-- This is my first html documentation. -->"
                "<!DOCTYPE html>"
                "\n"
                "<html>"
                "<head>"
                "<style>"
                "h1{"
                "text-align: center;"
                "color: rgb(185, 150, 105);"
                "font-family: \"Consolas\", \"Courier New\", monospace;"
                "}"
                "h3{"
                "text-align: center;"
                "color: rgb(0, 205, 80);"
                "font-family: \"Consolas\", \"Courier New\", monospace;"
                "}"
                "a:link{"
                "text-decoration: none;"
                "}"
                "a:hover{"
                "color : red;"
                "text-decoration: none;"
                "}"
                "</style>"
                "</head>"
                "</style>"
                "<body style=\"background: rgb(28, 28, 28);\">"
                "<h1> Ced </h1>"
                "<h3>"
                "Ced is a programm intended to increase your typing speed. Totaly written in C, using OpenGL as a renderer. <br>"
                "The platform layer written in Win32 to achieve high performance. <br>"
                "It is still in development. So for bug reports I suggest you to write on this email address: <br>"
                "<h3 style=\"color: #c1ffc1;\"> Alexey.Yevtushenko.Work@gmail.com </h3>"
                "</h3>"
                "</body>"
                "</html>");

        fclose(file);
    }
    return(0);
}
