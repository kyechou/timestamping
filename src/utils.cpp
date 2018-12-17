#include <string.h>
#include <locale.h>

const char *strerr(int errnum)
{
	const char *msg;
	locale_t locale;

	locale = newlocale(LC_ALL_MASK, "", 0);
	msg = strerror_l(errnum, locale);
	freelocale(locale);
	return msg;
}
