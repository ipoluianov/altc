/* AltC's addition to the startup of static executables, linked into every
   program: musl's static startup does not apply the IRELATIVE relocations
   of IFUNC symbols (the ifunc attribute, target_clones), which the linker
   puts between __rela_iplt_start and __rela_iplt_end. They are applied by
   the first constructor, before any other code of the program runs. */

typedef struct {
	unsigned long offset, info;
	long addend;
} __altc_rela;

extern const __altc_rela __rela_iplt_start[] __attribute__((weak, visibility("hidden")));
extern const __altc_rela __rela_iplt_end[] __attribute__((weak, visibility("hidden")));

#define R_X86_64_IRELATIVE 37

__attribute__((constructor(0), used)) static void __altc_apply_irelative(void) {
	for (const __altc_rela *r = __rela_iplt_start; r < __rela_iplt_end; r++) {
		if ((unsigned)r->info != R_X86_64_IRELATIVE)
			continue;
		unsigned long (*resolver)(void) = (unsigned long (*)(void))r->addend;
		*(unsigned long *)r->offset = resolver();
	}
}
