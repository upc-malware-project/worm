#pragma once

char *findSubstring(char* string, char *substr);

char *concat(const char *s1, const char *s2);

// To fix "global" variable, issue with loading crashes
void init_string_utils(Globals * glob);