# sdachi

This is an SNES Disassembler.

## Usage
`sdachi [options] <rom>`

## Command line options

### -a (--a)

Set the A register to 16 bits.

### -x (--x)

Set the X/Y register to 16 bits.

### -p (--pc)

Set disassemble start address in SNES Address.

**e.g.** `-p 0x008020`

When omitted, use reset vector.

### -r (--recursive)

Specify analysis depth of subroutine.

When you specify `-r 0`, it analyzes indefinitely.

### -c (--count)

Enable to data output mode.

And, specify the number of data output.

### -s (--split)

Specify the number of bytes up to the line feed.

### -l (--label)

Specify the label name for data output mode.

**e.g.** `-l FOO_DATA`

### -u (--upper)

Enable upper case outputs.

### -o (--output)

Specify the output file name.

When it is omitted, it becomes *<rom>.asm*.

### -v (--version)

Show version info.

### -? (--help)

Show usage.

## Thanks

Using the icon made by [Irasutoya(いらすとや)](http://irasutoya.com).

