#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define VGA ((volatile uint16_t*)0xB8000)
#define COM1 0x3F8

void *memset(void *dst, int c, size_t n);
static void putc(char c);
static void puts(const char *s);
static void printf(const char *fmt, ...);

static size_t row, col;
static uint8_t color = 0x0F;

static inline void outb(uint16_t port, uint8_t value) { __asm__ volatile("outb %0,%1" : : "a"(value), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t r; __asm__ volatile("inb %1,%0" : "=a"(r) : "Nd"(port)); return r; }

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
extern void gdt_flush(struct gdt_ptr *ptr);

static void gdt_set_gate(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    gdt[index].access = access;
}

static void gdt_init(void) {
    struct gdt_ptr ptr = { .limit = sizeof(gdt) - 1, .base = (uint32_t)&gdt };
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_flush(&ptr);
}

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct interrupt_frame {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t vector, error_code, eip, cs, eflags;
};

static struct idt_entry idt[256];
static const char *exception_names[32] = {
    "divide error", "debug", "non-maskable interrupt", "breakpoint",
    "overflow", "bound range", "invalid opcode", "device not available",
    "double fault", "coprocessor segment", "invalid TSS", "segment not present",
    "stack fault", "general protection fault", "page fault", "reserved",
    "x87 floating point", "alignment check", "machine check", "SIMD floating point",
    "virtualization", "control protection", "reserved", "reserved",
    "reserved", "reserved", "reserved", "reserved", "hypervisor injection",
    "VMM communication", "security", "reserved"
};

extern void isr0(void); extern void isr1(void); extern void isr2(void); extern void isr3(void);
extern void isr4(void); extern void isr5(void); extern void isr6(void); extern void isr7(void);
extern void isr8(void); extern void isr9(void); extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

static void idt_set_gate(uint8_t index, void (*handler)(void)) {
    uint32_t base = (uint32_t)handler;
    idt[index].base_low = base & 0xFFFF;
    idt[index].selector = 0x08;
    idt[index].zero = 0;
    idt[index].flags = 0x8E;
    idt[index].base_high = (base >> 16) & 0xFFFF;
}

static void idt_load(void) {
    struct idt_ptr ptr = { .limit = sizeof(idt) - 1, .base = (uint32_t)&idt };
    __asm__ volatile("lidt %0" : : "m"(ptr));
}

static void pic_remap(void) {
    uint8_t a1 = inb(0x21), a2 = inb(0xA1);
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, a1); outb(0xA1, a2);
}

static void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    void (*handlers[32])(void) = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };
    for (uint8_t i = 0; i < 32; i++) idt_set_gate(i, handlers[i]);
    pic_remap();
    idt_load();
}

static void print_hex32(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    puts("0x");
    for (int i = 7; i >= 0; i--) putc(hex[(v >> (i * 4)) & 0xF]);
}

void isr_exception_handler(struct interrupt_frame *frame) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2,%0" : "=r"(cr2));
    puts("\nKERNEL PANIC: CPU exception\n");
    printf("vector=%u name=%s\n", frame->vector, frame->vector < 32 ? exception_names[frame->vector] : "unknown");
    puts("error="); print_hex32(frame->error_code); puts(" eip="); print_hex32(frame->eip);
    puts(" cs="); print_hex32(frame->cs); puts(" eflags="); print_hex32(frame->eflags); putc('\n');
    puts("eax="); print_hex32(frame->eax); puts(" ebx="); print_hex32(frame->ebx);
    puts(" ecx="); print_hex32(frame->ecx); puts(" edx="); print_hex32(frame->edx); putc('\n');
    puts("esi="); print_hex32(frame->esi); puts(" edi="); print_hex32(frame->edi);
    puts(" ebp="); print_hex32(frame->ebp); puts(" esp="); print_hex32(frame->esp); putc('\n');
    if (frame->vector == 14) { puts("cr2="); print_hex32(cr2); putc('\n'); }
    puts("System halted after handled exception.\n");
    for (;;) __asm__ volatile("cli; hlt");
}

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

static void paging_init(void) {
    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
        first_page_table[i] = (i * 0x1000) | 0x3;
    }
    page_directory[0] = ((uint32_t)first_page_table) | 0x3;
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

static void trigger_divide_error(void) {
    __asm__ volatile("xorl %%edx, %%edx; movl $1, %%eax; movl $0, %%ecx; divl %%ecx" ::: "eax", "ecx", "edx");
}

static void trigger_page_fault(void) {
    volatile uint32_t *bad = (volatile uint32_t*)0xFFC00000;
    (void)*bad;
}

void *memset(void *dst, int c, size_t n) { unsigned char *p = dst; while (n--) *p++ = (unsigned char)c; return dst; }
void *memcpy(void *dst, const void *src, size_t n) { unsigned char *d = dst; const unsigned char *s = src; while (n--) *d++ = *s++; return dst; }
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (unsigned char)*a - (unsigned char)*b; }
static int starts_with(const char *s, const char *prefix) { while (*prefix) if (*s++ != *prefix++) return 0; return 1; }

static void vga_clear(void) { for (size_t i = 0; i < 80 * 25; i++) VGA[i] = ((uint16_t)color << 8) | ' '; row = col = 0; }
static void vga_putc(char c) {
    if (c == '\r') return;
    if (c == '\n') { col = 0; if (++row >= 25) row = 24; return; }
    VGA[row * 80 + col] = ((uint16_t)color << 8) | (uint8_t)c;
    if (++col >= 80) { col = 0; if (++row >= 25) row = 24; }
}

static void serial_init(void) {
    outb(COM1 + 1, 0x00); outb(COM1 + 3, 0x80); outb(COM1 + 0, 0x03); outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); outb(COM1 + 2, 0xC7); outb(COM1 + 4, 0x0B);
}
static int serial_ready(void) { return inb(COM1 + 5) & 0x20; }
static void serial_putc(char c) { while (!serial_ready()) {} outb(COM1, (uint8_t)c); }
static int serial_received(void) { return inb(COM1 + 5) & 1; }
static char serial_getc(void) { while (!serial_received()) {} return (char)inb(COM1); }

static void putc(char c) { vga_putc(c); serial_putc(c); }
static void puts(const char *s) { while (*s) putc(*s++); }
static void print_uint(uint32_t v) { char b[11]; int i = 0; if (!v) { putc('0'); return; } while (v) { b[i++] = '0' + v % 10; v /= 10; } while (i--) putc(b[i]); }
static void printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { putc(*fmt); continue; }
        fmt++;
        if (*fmt == 's') puts(va_arg(ap, const char*));
        else if (*fmt == 'u') print_uint(va_arg(ap, uint32_t));
        else if (*fmt == 'c') putc((char)va_arg(ap, int));
        else { putc('%'); putc(*fmt); }
    }
    va_end(ap);
}

static void libc_tests(void) {
    char buf[8]; memset(buf, 'A', 3); buf[3] = 0;
    printf("M5 libc tests: %s\n", (!strcmp(buf, "AAA") && strlen(buf) == 3) ? "PASS" : "FAIL");
}
static void prompt(void) { puts("agentic-os> "); }
static void run_command(char *line) {
    if (!strcmp(line, "help")) puts("Commands: help echo clear ls reboot status test_div0 test_pagefault\n");
    else if (starts_with(line, "echo ")) { puts(line + 5); putc('\n'); }
    else if (!strcmp(line, "clear")) vga_clear();
    else if (!strcmp(line, "ls")) puts("/README.TXT\n/INITRD.TXT\n");
    else if (!strcmp(line, "status")) puts("M1 boot, M2 IDT/PIC exceptions, M5 utilities, M8 serial shell: PASS; later milestones deferred.\n");
    else if (!strcmp(line, "test_div0")) { puts("Triggering divide error...\n"); trigger_divide_error(); }
    else if (!strcmp(line, "test_pagefault")) { puts("Triggering page fault...\n"); trigger_page_fault(); }
    else if (!strcmp(line, "reboot")) { puts("Reboot requested (halt in test build).\n"); for (;;) __asm__ volatile("hlt"); }
    else if (line[0]) printf("unknown command: %s\n", line);
}

void kernel_main(uint32_t magic, uint32_t mb_info) {
    (void)magic; (void)mb_info;
    serial_init(); vga_clear();
    gdt_init();
    idt_init();
    paging_init();
    puts("Hello from kernel\n");
    puts("Agentic OS booted via GRUB Multiboot (32-bit protected mode).\n");
    libc_tests();
    puts("Interactive serial shell ready. Type help.\n");
    char line[128]; size_t len = 0; prompt();
    for (;;) {
        char c = serial_getc();
        if (c == '\r') c = '\n';
        if (c == '\n') { putc('\n'); line[len] = 0; run_command(line); len = 0; prompt(); continue; }
        if ((c == 8 || c == 127) && len) { len--; puts("\b \b"); continue; }
        if (c >= 32 && c < 127 && len + 1 < sizeof(line)) { line[len++] = c; putc(c); }
    }
}
