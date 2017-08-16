# udis86 - scripts/ud_opcode.py
# 
# Copyright (c) 2009, 2013 Vivek Thampi
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice, 
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, 
#       this list of conditions and the following disclaimer in the documentation 
#       and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os

# Some compatibility stuff for supporting python 2.x as well as python 3.x
def itemslist(dict):
    try:
        return dict.iteritems() # python 2.x
    except AttributeError:
        return list(dict.items()) # python 3.x

class UdInsnDef:
    """An x86 instruction definition
    """
    def __init__(self, **insnDef):
        self.mnemonic  = insnDef['mnemonic']
        self.prefixes  = insnDef['prefixes']
        self.opcodes   = insnDef['opcodes']
        self.operands  = insnDef['operands']
        self._cpuid    = insnDef['cpuid']
        self._opcexts  = {}

        for opc in self.opcodes:
            if opc.startswith('/'):
                e, v = opc.split('=')
                self._opcexts[e] = v

    def lookupPrefix(self, pfx):
        """Lookup prefix (if any, None otherwise), by name"""
        return True if pfx in self.prefixes else None


    @property
    def vendor(self):
        return self._opcexts.get('/vendor', None)

    @property
    def mode(self):
        return self._opcexts.get('/m', None)

    @property
    def osize(self):
        return self._opcexts.get('/o', None)

    def isDef64(self):
        return 'def64' in self.prefixes

    def __str__(self):
        return self.mnemonic + " " + ', '.join(self.operands) + \
               " " + ' '.join(self.opcodes)


class UdOpcodeTable:
    """A single table of instruction definitions, indexed by
       a decode field. 
    """

    class CollisionError(Exception):
        pass

    class IndexError(Exception):
        """Invalid Index Error"""
        pass

    @classmethod
    def vendor2idx(cls, v):
        return (0 if v == 'amd' 
                  else (1 if v == 'intel'
                          else 2))

    @classmethod
    def vex2idx(cls, v):
        if v.startswith("none_"):
            v = v[5:]
        vexOpcExtMap = {
            'none'      : 0x0, 
            '0f'        : 0x1, 
            '0f38'      : 0x2, 
            '0f3a'      : 0x3,
            '66'        : 0x4, 
            '66_0f'     : 0x5, 
            '66_0f38'   : 0x6, 
            '66_0f3a'   : 0x7,
            'f3'        : 0x8, 
            'f3_0f'     : 0x9, 
            'f3_0f38'   : 0xa, 
            'f3_0f3a'   : 0xb,
            'f2'        : 0xc, 
            'f2_0f'     : 0xd, 
            'f2_0f38'   : 0xe, 
            'f2_0f3a'   : 0xf,
        }
        return vexOpcExtMap[v]


    # A mapping of opcode extensions to their representational
    # values used in the opcode map.
    OpcExtMap = {
        '/rm'    : lambda v: int(v, 16),
        '/x87'   : lambda v: int(v, 16),
        '/3dnow' : lambda v: int(v, 16),
        '/reg'   : lambda v: int(v, 16),
        # modrm.mod
        # (!11, 11)    => (00b, 01b)
        '/mod'   : lambda v: 0 if v == '!11' else 1,
        # Mode extensions:
        # (16, 32, 64) => (00, 01, 02)
        '/o'     : lambda v: (int(v) / 32),
        '/a'     : lambda v: (int(v) / 32),
        # Disassembly mode 
        # (!64, 64)    => (00b, 01b)
        '/m'     : lambda v: 1 if v == '64' else 0,
        # SSE
        # none => 0
        # f2   => 1
        # f3   => 2
        # 66   => 3
        '/sse'   : lambda v: (0 if v == 'none'
                                else (((int(v, 16) & 0xf) + 1) / 2)),
        # AVX
        '/vex'   : lambda v: UdOpcodeTable.vex2idx(v),
        '/vexw'  : lambda v: 0 if v == '0' else 1,
        '/vexl'  : lambda v: 0 if v == '0' else 1,
        # Vendor
        '/vendor': lambda v: UdOpcodeTable.vendor2idx(v)
    }


    _TableInfo = {
        'opctbl'    : { 'label' : 'UD_TAB__OPC_TABLE',   'size' : 256 },
        '/sse'      : { 'label' : 'UD_TAB__OPC_SSE',     'size' : 4 },
        '/reg'      : { 'label' : 'UD_TAB__OPC_REG',     'size' : 8 },
        '/rm'       : { 'label' : 'UD_TAB__OPC_RM',      'size' : 8 },
        '/mod'      : { 'label' : 'UD_TAB__OPC_MOD',     'size' : 2 },
        '/m'        : { 'label' : 'UD_TAB__OPC_MODE',    'size' : 2 },
        '/x87'      : { 'label' : 'UD_TAB__OPC_X87',     'size' : 64 },
        '/a'        : { 'label' : 'UD_TAB__OPC_ASIZE',   'size' : 3 },
        '/o'        : { 'label' : 'UD_TAB__OPC_OSIZE',   'size' : 3 },
        '/3dnow'    : { 'label' : 'UD_TAB__OPC_3DNOW',   'size' : 256 },
        '/vendor'   : { 'label' : 'UD_TAB__OPC_VENDOR',  'size' : 3 },
        '/vex'      : { 'label' : 'UD_TAB__OPC_VEX',     'size' : 16 },
        '/vexw'     : { 'label' : 'UD_TAB__OPC_VEX_W',   'size' : 2 },
        '/vexl'     : { 'label' : 'UD_TAB__OPC_VEX_L',   'size' : 2 },
    }


    def __init__(self, typ):
        assert typ in self._TableInfo
        self._typ     = typ
        self._entries = {}


    def size(self):
        return self._TableInfo[self._typ]['size']

    def entries(self):
        return itemslist(self._entries)

    def numEntries(self):
        return len(self._entries.keys())

    def label(self):
        return self._TableInfo[self._typ]['label']

    def typ(self):
        return self._typ

    def meta(self):
        return self._typ


    def __str__(self):
        return "table-%s" % self._typ


    def add(self, opc, obj):
        typ = UdOpcodeTable.getOpcodeTyp(opc)
        idx = UdOpcodeTable.getOpcodeIdx(opc)
        if self._typ != typ or idx in self._entries:
            raise CollisionError()
        self._entries[idx] = obj


    def lookup(self, opc):
        typ = UdOpcodeTable.getOpcodeTyp(opc)
        idx = UdOpcodeTable.getOpcodeIdx(opc)
        if self._typ != typ:
            raise UdOpcodeTable.CollisionError("%s <-> %s" % (self._typ, typ))
        return self._entries.get(idx, None)

    
    def entryAt(self, index):
        """Returns the entry at a given index of the table,
           None if there is none. Raises an exception if the
           index is out of bounds.
        """
        if index < self.size():
            return self._entries.get(index, None)
        raise self.IndexError("index out of bounds: %s" % index)

    def setEntryAt(self, index, obj):
        if index < self.size():
            self._entries[index] = obj
        else:
            raise self.IndexError("index out of bounds: %s" % index)

    @classmethod
    def getOpcodeTyp(cls, opc):
        if opc.startswith('/'):
            return opc.split('=')[0]
        else:
            return 'opctbl'


    @classmethod
    def getOpcodeIdx(cls, opc):
        if opc.startswith('/'):
            typ, v = opc.split('=')
            return cls.OpcExtMap[typ](v)
        else:
            # plain opctbl opcode
            return int(opc, 16)


    @classmethod
    def getLabels(cls):
        """Returns a list of all labels"""
        return [cls._TableInfo[k]['label'] for k in cls._TableInfo.keys()]


class UdOpcodeTables(object):
    """Collection of opcode tables
    """

    class CollisionError(Exception):
        def __init__(self, obj1, obj2):
            self.obj1, self.obj2 = obj1, obj2

    def newTable(self, typ):
        """Create a new opcode table of a give type `typ`. """
        tbl = UdOpcodeTable(typ)
        self._tables.append(tbl)
        return tbl

    def mkTrie(self, opcodes, obj):
        """Recursively contruct a trie entry mapping a string of
           opcodes to an object.
        """
        if len(opcodes) == 0:
            return obj
        opc = opcodes[0]
        tbl = self.newTable(UdOpcodeTable.getOpcodeTyp(opc))
        tbl.add(opc, self.mkTrie(opcodes[1:], obj))
        return tbl

    def walk(self, tbl, opcodes):
        """Walk down the opcode trie, starting at a given opcode
           table, given a string of opcodes. Return None if unable
           to walk, the object at the leaf otherwise.
        """
        opc = opcodes[0]
        e   = tbl.lookup(opc)
        if e is None:
            return None
        elif isinstance(e, UdOpcodeTable) and len(opcodes[1:]):
            return self.walk(e, opcodes[1:])
        return e

    def map(self, tbl, opcodes, obj):
        """Create a mapping from a given string of opcodes to an
           object in the opcode trie. Constructs trie branches as
           needed.
        """
        opc = opcodes[0]
        e   =  tbl.lookup(opc)
        if e is None:
            tbl.add(opc, self.mkTrie(opcodes[1:], obj))
        else:
            if len(opcodes[1:]) == 0:
                raise self.CollisionError(e, obj)
            self.map(e, opcodes[1:], obj)

    def __init__(self, xml):
        self._tables    = []
        self._insns     = []
        self._mnemonics = {}

        # The root table is always a 256 entry opctbl, indexed
        # by a plain opcode byte
        self.root       = self.newTable('opctbl')

        if os.getenv("UD_OPCODE_DEBUG"):
            self._logFh     = open("opcodeTables.log", "w")

        # add an invalid instruction entry without any mapping
        # in the opcode tables.
        self.invalidInsn = UdInsnDef(mnemonic="invalid", opcodes=[], cpuid=[],
                                     operands=[], prefixes=[])
        self._insns.append(self.invalidInsn)

        # Construct UdOpcodeTables object from the given
        # udis86 optable.xml
        for insn in self.__class__.parseOptableXML(xml):
            self.addInsnDef(insn)
        self.patchAvx2byte()
        self.mergeSSENONE()
        self.printStats()

    def log(self, s):
        if os.getenv("UD_OPCODE_DEBUG"):
            self._logFh.write(s + "\n")


    def mergeSSENONE(self):
        """Merge sse tables with only one entry for /sse=none
        """
        for table in self._tables:
            for k, e in table.entries():
                if isinstance(e, UdOpcodeTable) and e.typ() == '/sse':
                    if e.numEntries() == 1:
                        sse = e.lookup("/sse=none")
                        if sse:
                            table.setEntryAt(k, sse)
        uniqTables = {}
        def genTableList(tbl):
            if tbl not in uniqTables:
                self._tables.append(tbl)
            uniqTables[tbl] = 1
            for k, e in tbl.entries():
                if isinstance(e, UdOpcodeTable):
                    genTableList(e)
        self._tables = []
        genTableList(self.root)
                

    def patchAvx2byte(self):
        # create avx tables
        for pp in (None, 'f2', 'f3', '66'):
            for m in (None, '0f', '0f38', '0f3a'):
                if pp is None and m is None:
                    continue
                if pp is None:
                    vex = m
                elif m is None:
                    vex = pp
                else:
                    vex = pp + '_' + m
                table = self.walk(self.root, ('c4', '/vex=' + vex))
                self.map(self.root, ('c5', '/vex=' + vex), table)


    def addInsn(self, **insnDef):

        # Canonicalize opcode list
        opcexts = insnDef['opcexts']
        opcodes = list(insnDef['opcodes'])

        # Re-order vex
        if '/vex' in opcexts:
            assert opcodes[0] == 'c4' or opcodes[0] == 'c5'
            opcodes.insert(1, '/vex=' + opcexts['/vex'])

        # Add extensions. The order is important, and determines how
        # well the opcode table is packed. Also note, /sse must be
        # before /o, because /sse may consume operand size prefix
        # affect the outcome of /o.
        for ext in ('/mod', '/x87', '/reg', '/rm', '/sse', '/o', '/a', '/m',
                    '/vexw', '/vexl', '/3dnow', '/vendor'):
            if ext in opcexts:
                opcodes.append(ext + '=' + opcexts[ext])

        insn = UdInsnDef(mnemonic = insnDef['mnemonic'],
                         prefixes = insnDef['prefixes'],
                         operands = insnDef['operands'],
                         opcodes  = opcodes,
                         cpuid    = insnDef['cpuid'])
        try:
            self.map(self.root, opcodes, insn)
        except self.CollisionError as e:
            self.pprint()
            print(opcodes, insn, str(e.obj1), str(e.obj2))
            raise
        except Exception as e:
            self.pprint()
            raise
        self._insns.append(insn)
        # add to lookup by mnemonic structure
        if insn.mnemonic not in self._mnemonics:
            self._mnemonics[insn.mnemonic] = [ insn ]
        else:
            self._mnemonics[insn.mnemonic].append(insn)


    def addInsnDef(self, insnDef):
        opcodes  = []
        opcexts  = {}

        # pack plain opcodes first, and collect opcode
        # extensions
        for opc in insnDef['opcodes']:
            if not opc.startswith('/'):
                opcodes.append(opc)
            else:
                e, v = opc.split('=')
                opcexts[e] = v

        # treat vendor as an opcode extension
        if len(insnDef['vendor']):
            opcexts['/vendor'] = insnDef['vendor'][0]

        if insnDef['mnemonic'] in ('lds', 'les'):
            #
            # Massage lds and les, which share the same prefix as AVX
            # instructions, to work well with the opcode tree.
            #
            opcexts['/vex'] = 'none'
        elif '/vex' in opcexts:
            # A proper avx instruction definition; make sure there are
            # no legacy opcode extensions
            assert '/sse' not in opcodes

            # make sure the opcode definitions don't already include
            # the avx prefixes.
            assert opcodes[0] not in ('c4', 'c5')

            # An avx only instruction is defined by the /vex= opcode
            # extension. They do not include the c4 (long form) or
            # c5 (short form) prefix. As part of opcode table generate,
            # here we create the long form definition, and then patch
            # the table for c5 in a later stage.
            # Construct a long-form definition of the avx instruction
            opcodes.insert(0, 'c4')
        elif (opcodes[0] == '0f' and opcodes[1] != '0f' and
            '/sse' not in opcexts):
            # Make all 2-byte opcode form isntructions play nice with sse
            # opcode maps.
            opcexts['/sse'] = 'none'

        # legacy sse defs that get promoted to avx
        fn = self.addInsn
        if 'avx' in insnDef['cpuid'] and '/sse' in opcexts:
            fn = self.addSSE2AVXInsn

        fn(mnemonic = insnDef['mnemonic'],
           prefixes = insnDef['prefixes'],
           opcodes  = opcodes,
           opcexts  = opcexts,
           operands = insnDef['operands'],
           cpuid    = insnDef['cpuid'])


    def addSSE2AVXInsn(self, **insnDef):
        """Add an instruction definition containing an avx cpuid bit, but
           declared in its legacy SSE form. The function splits the
           definition to create two new definitions, one for SSE and one
           promoted to an AVX form.
        """

        # SSE
        ssemnemonic = insnDef['mnemonic']
        sseopcodes  = insnDef['opcodes']
        # remove vex opcode extensions
        sseopcexts  = dict([(e, v) for e, v in itemslist(insnDef['opcexts'])
                                  if not e.startswith('/vex')])
        # strip out avx operands, preserving relative ordering
        # of remaining operands
        sseoperands = [opr for opr in insnDef['operands']
                        if opr not in ('H', 'L')]
        # strip out avx prefixes
        sseprefixes = [pfx for pfx in insnDef['prefixes']
                        if not pfx.startswith('vex')]
        # strip out avx bits from cpuid
        ssecpuid    = [flag for flag in insnDef['cpuid']
                        if not flag.startswith('avx')]

        self.addInsn(mnemonic = ssemnemonic,
                     prefixes = sseprefixes,
                     opcodes  = sseopcodes,
                     opcexts  = sseopcexts,
                     operands = sseoperands,
                     cpuid    = ssecpuid)

        # AVX
        vexmnemonic = 'v' + insnDef['mnemonic']
        vexprefixes = insnDef['prefixes']
        vexopcodes  = ['c4']
        vexopcexts  = dict([(e, insnDef['opcexts'][e])
                              for e in insnDef['opcexts'] if e != '/sse'])
        vexopcexts['/vex'] = insnDef['opcexts']['/sse'] + '_' + '0f'
        if insnDef['opcodes'][1] == '38' or insnDef['opcodes'][1] == '3a':
            vexopcexts['/vex'] += insnDef['opcodes'][1]
            vexopcodes.extend(insnDef['opcodes'][2:])
        else:
            vexopcodes.extend(insnDef['opcodes'][1:])
        vexoperands = []
        for o in insnDef['operands']:
            # make the operand size explicit: x
            if o in ('V', 'W', 'H', 'U'):
                o = o + 'x'
            vexoperands.append(o)
        vexcpuid    = [flag for flag in insnDef['cpuid']
                        if not flag.startswith('sse')]

        self.addInsn(mnemonic = vexmnemonic,
                     prefixes = vexprefixes,
                     opcodes  = vexopcodes,
                     opcexts  = vexopcexts,
                     operands = vexoperands,
                     cpuid    = vexcpuid)

    def getInsnList(self):
        """Returns a list of all instructions in the collection"""
        return self._insns


    def getTableList(self):
        """Returns a list of all tables in the collection"""
        return self._tables

    def getMnemonicsList(self):
        """Returns a sorted list of mnemonics"""
        return sorted(self._mnemonics.keys())


    def pprint(self):
        def printWalk(tbl, indent=""):
            entries = tbl.entries()
            for k, e in entries:
                if isinstance(e, UdOpcodeTable):
                    self.log("%s    |-<%02x> %s" % (indent, k, e))
                    printWalk(e, indent + "    |")
                elif isinstance(e, UdInsnDef):
                    self.log("%s    |-<%02x> %s" % (indent, k, e))
        printWalk(self.root)


    def printStats(self):
        tables = self.getTableList()
        self.log("stats: ")
        self.log("  Num tables    = %d" % len(tables))
        self.log("  Num insnDefs  = %d" % len(self.getInsnList()))
        self.log("  Num insns     = %d" % len(self.getMnemonicsList()))

        totalSize = 0
        totalEntries = 0
        for table in tables:
            totalSize += table.size()
            totalEntries += table.numEntries()
        self.log("  Packing Ratio = %d%%" % ((totalEntries * 100) / totalSize))
        self.log("--------------------")

        self.pprint()


    @staticmethod
    def parseOptableXML(xml):
        """Parse udis86 optable.xml file and return list of
           instruction definitions.
        """
        from xml.dom import minidom

        xmlDoc = minidom.parse(xml)
        tlNode = xmlDoc.firstChild
        insns  = []

        while tlNode and tlNode.localName != "x86optable": 
            tlNode = tlNode.nextSibling

        for insnNode in tlNode.childNodes:
            if not insnNode.localName:
                continue
            if insnNode.localName != "instruction":
                raise Exception("warning: invalid insn node - %s" % insnNode.localName)
            mnemonic = insnNode.getElementsByTagName('mnemonic')[0].firstChild.data
            vendor, cpuid = '', []

            for node in insnNode.childNodes:
                if node.localName == 'vendor':
                    vendor = node.firstChild.data.split()
                elif node.localName == 'cpuid':
                    cpuid = node.firstChild.data.split()

            for node in insnNode.childNodes:
                if node.localName == 'def':
                    insnDef = { 'pfx' : [] }
                    for node in node.childNodes:
                        if not node.localName:
                            continue
                        if node.localName in ('pfx', 'opc', 'opr', 'vendor', 'cpuid'):
                            insnDef[node.localName] = node.firstChild.data.split()
                        elif node.localName == 'mode':
                            insnDef['pfx'].extend(node.firstChild.data.split())
                    insns.append({'prefixes' : insnDef.get('pfx', []),
                                  'mnemonic' : mnemonic,
                                  'opcodes'  : insnDef.get('opc', []),
                                  'operands' : insnDef.get('opr', []),
                                  'vendor'   : insnDef.get('vendor', vendor),
                                  'cpuid'    : insnDef.get('cpuid', cpuid)})
        return insns
