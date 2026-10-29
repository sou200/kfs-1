#ifndef CMD_H
#define CMD_H

int strcmp(const char *s1, const char *s2);
void handle_keypress(char ascii);
void execute_command(char *cmd);

#endif