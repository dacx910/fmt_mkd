# fmt_mkd
Strips multi-line codeblocks and Hugo metadata from Markdown files for more accurate word counts.

Relatively useless utility, but built without standard libraries and statically linked. Code re-used from [nolibc](https://github.com/wtarreau/nolibc/) in multiple parts, modified for my use-case. Because of these modifications, this is extremely non-portable (targeting x86_64 Linux).

However, it uses a very low number of syscalls (except for the awful `printf()` implementation) and the code footprint is much smaller than a standard utility.