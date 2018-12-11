#!/bin/bash

type astyle >/dev/null 2>&1 || (echo '[!] astyle is not installed.' >&2; exit 1)

usage()
{
	echo "[!] Usage: $0 [OPTIONS]" >&2
	echo '    Options:' >&2
	echo '        -a    automatically update the source files' >&2
	echo '        -h    show this message and exit' >&2
}

AUTO=0
while getopts 'ah' op; do
	case $op in
		a)
			AUTO=1 ;;
		h|*)
			usage
			exit 1 ;;
	esac
done

EXITCODE=0
ASTYLE_OPTS=(
	'--style=kr'
	'--indent=tab=8'
	'--pad-oper'
	'--pad-comma'
	'--pad-header'
	'--align-pointer=name'
	'--suffix=none'
)
SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
FILES=$(find ${SCRIPT_DIR}/../src ${SCRIPT_DIR} -type f | grep -E '\.(c|h)$')

for FILE in $FILES; do
	newfile="$(mktemp "tmp.XXXXXX")" || exit 1
	if ! astyle ${ASTYLE_OPTS[*]} <"$FILE" >"$newfile"; then
		echo >&2
		echo "[!] astyle failed." >&2
		exit 1
	fi
	if ! diff -upB "$FILE" "$newfile" &>/dev/null; then
		if [ $AUTO -ne 0 ]; then
			cp "$newfile" "$FILE"
			echo >&2
			echo "[!] $FILE does not follow the coding style (file changed)." >&2
		else
			echo >&2
			echo "[!] $FILE does not follow the coding style." >&2
			echo '    Please run:' >&2
			echo "        astyle ${ASTYLE_OPTS[*]} $FILE" >&2
		fi
		EXITCODE=1
	fi
	rm -f "$newfile"
done

exit $EXITCODE
