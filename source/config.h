#ifndef CONFIG_H
#define CONFIG_H

void default_config();
void default_theme();

bool parse_config();
bool save_config();
bool parse_theme(char*);

bool get_theme_file(const char*, const char*, char*);

#endif //CONFIG_H
