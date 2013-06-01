#pragma once

unsigned int a2w(const char * astr, wchar_t *wstr);

unsigned int w2a(const wchar_t *wstr, char *astr);

unsigned int a2u8(const char *astr, char *utf8);

unsigned int u82a(const char *utf8, char *astr);

unsigned int u2u8(const wchar_t *wstr, char *utf8);

unsigned int u82u(const char *utf8, wchar_t *wstr);

