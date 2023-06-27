#! /usr/bin/python3

import sys
import struct
import json
import binascii
import re
import copy

sys.path.append("../Projects/STM32F429I-Discovery/Applications/au-taf/Src")
import api

re_str = re.compile("string\[(\d+)\]")

# Order matters as 'command' is optional
# Should match enum in au-taf/Inc/pico.h file
WORDS = ["next", "ok", "back", "command"]


def htobin(utf8):
    binary = b''
    text = utf8.decode("utf-8")
    i = 0
    while True:
        im = text.find("0x",i)
        iM = text.find("0X",i)
        if im == -1 and iM == -1:
            break
        elif im == -1:
            i = iM
        elif iM == -1:
            i = im
        elif im < iM:
            i = im
        else:
            i = iM
        # end if
        binary += bytes([int(text[i:i+4],0)])
        i += 3
    # end while
    return binary

def force_len(text,l):
    if len(text) > l:
        text = text[0:l]
    elif (len(text) < l):
        text += "\x00" * (l - len(text))
    # end if
    return text.encode("utf-8")

class checker:
    def __init__(self,id_max):
        self.current_level = 0
        self.choices = []
        self.current_menu = 0
        self.current_choice = 0
        self.action_id = 0
        self.id_max = id_max

    def push(self,new_menu,choice_id):
        self.choices.append([self.current_menu,self.current_choice,self.action_id,choice_id])
        self.action_id = self.get_action_id(choice_id)
        self.current_menu = new_menu
        self.current_choice = 0
        self.current_level += 1
        
    def pop(self):
        [self.current_menu,self.current_choice,self.action_id,choice_id] = self.choices.pop()
        self.current_level -= 1

    def get_action_id(self,choice_id):
        return self.action_id * (self.id_max + 1) + choice_id + 1
        
class generator:
    def __init__(self,config_data):
        self.veille = True
        self.config_data = config_data
        self.strings_to_index = dict()
        self.current_choice = 0
        self.current_first_choice = 0
        self.choices = [0]*4
        self.current_choice_level = 0
        self.action_id = 0
        self.current_menu_offset = 0
        self.generated = False
        self.c_dict = config_data["CONFIG"]


    def add_string(self,string):
        if string in self.strings_to_index.keys():
            return self.strings_to_index[string]
        # end if
        n = len(self.strings_to_index)
        self.strings_to_index.update({string:n})
        return n

    def compute_id_from_choices(self,name):
        words = name.split("@")
        words.pop(0)
        v = 0
        for word in words:
            if word not in self.strings_to_index.keys():
                print("Missing word %s" % (word))
                raise
            else:
                v = v * (self.strings_number+1) + (self.strings_to_index[word]+1)
            # end if
        # end for
        return v
    
    def generate(self):
        if self.generated:
            return
        # end if


        if "CONFIG" in self.config_data.keys():
            if "picovoice" in self.config_data["CONFIG"].keys():
              
                self.picovoice_key = "{YourPicoveAccessKey}"
                if "key" in self.config_data["CONFIG"]["picovoice"].keys():
                    self.picovoice_key = self.config_data["CONFIG"]["picovoice"]["key"]
                # end if
 
                if len(self.picovoice_key) % 16 != 0:
                    padding = 16 - (len(self.picovoice_key) % 16)
                    self.picovoice_key += "\x00" * padding
                # end if
                
                self.picovoice_language = "zh"
                if "language" in self.config_data["CONFIG"]["picovoice"].keys():
                    self.picovoice_language = self.config_data["CONFIG"]["picovoice"]["language"]
                # end if
                self.words = dict()
                for w in WORDS:
                    if w not in self.config_data["CONFIG"]["picovoice"].keys():
                        if w == "command":
                            continue
                        # end if
                        raise
                    # end if
                    name = self.config_data["CONFIG"]["picovoice"][w]["file"]
                    extension = name.split('.')[-1]
                    if extension == "h":
                        f = open(name,"rb")
                        data = htobin(f.read())
                        f.close()
                    elif extension == "ppn":
                        f = open(name,"rb")
                        data = f.read()
                        f.close()
                    else:
                        raise
                    # end if
                    
                    effective_length = len(data)
                    
                    if len(data) % 16 != 0:
                        padding = 16 - (len(data) % 16)
                        data += bytes([0] * padding)
                    # end if

                    # In the same order than the WORDS array
                    self.words[WORDS.index(w)] = {"threshold" : float(self.config_data["CONFIG"]["picovoice"][w]["threshold"]),
                                                  "data" : data,
                                                  "length" : effective_length}
                # end for
            # end if
        # end if


        menus = ["START"]
        menus_done = []

        menu_numbers = dict()
        self.menus = dict()

        menu_id = 0
        menu_to_id = dict()
        menu_offset = 0
        menu_to_offset = dict()
        actions = b''
        while len(menus) != 0:
            m = menus.pop(0)
            if m not in menus_done:
                if m not in self.config_data["MENUS"].keys():
                    print("Missing menu %s" % m)
                    continue
                # end if
                
                if m not in menu_to_id.keys():
                    menu_to_id.update({m:menu_id})
                    menu_id += 1
                # end if
                if m not in menu_to_offset.keys():
                    menu_to_offset.update({m:menu_offset})
                    size = 1 + 3 * len(self.config_data["MENUS"][m])
                    menu_offset += size
                # end if
                menu_actions = []
                menu_actions_length = 0
                menus_done.append(m)
                for item in self.config_data["MENUS"][m]:
                    action_start = len(actions)
                    menu_choice = []
                    string_index = self.add_string(item[0])
                    i = 1
                    while i < len(item):
                        # print(" Treating %s" % str(item[i]))
                        for action in item[i]:
                          if action[0] == "MENU":
                              submenu = action[1]
                              if submenu not in menus and submenu not in menus_done:
                                  menus.append(submenu)
                              # end if
                              if submenu in menu_to_id.keys():
                                  submenu_id = menu_to_id[submenu]
                              else:
                                  submenu_id = menu_id + menus.index(submenu)
                              # end if
                              if submenu not in menu_to_offset.keys():
                                  for a in menus:
                                      if a not in menu_to_offset.keys():
                                          menu_to_offset.update({a:menu_offset})
                                          size = 1 + 3 * len(self.config_data["MENUS"][a])
                                          menu_offset += size
                                      # end if
                                  # end for
                              # end if
                              submenu_offset = menu_to_offset[submenu]
                              actions += struct.pack("<BH",api.values["COMMANDS"]["MENU"],submenu_offset)
                              menu_choice.append(["MENU",submenu])
                          elif action[0] == "PHONE":
                              actions += bytes([api.values["COMMANDS"]["PHONE"][action[1]]])
                              if action[1] == "SMS":
                                  actions += bytes([len(action[2])])
                                  actions += action[2].encode("utf-8")
                                  actions += bytes([len(action[3])])
                                  actions += action[3].encode("utf-8")
                              elif action[1] == "CALL":
                                  actions += bytes([len(action[2])])
                                  actions += action[2].encode("utf-8")
                              elif action[1] in ["HANGUP", "VOL_H","VOL_L", "UNHOOK"]:
                                  pass
                              else:
                                  raise toto
                              # end if
                          elif action[0] == "RF_433":
                              try:
                                  code = int(action[2],base=0)
                              except Exception as e:
                                  code = int(self.c_dict[cmds[i+1]][cmds[i+2]],base=0)
                              # end try
                              actions += struct.pack("<BI",
                                                     api.values["COMMANDS"]["RF_433"][action[1]],
                                                     code)
                          elif action[0] == "INFRA_RED":
                              try:
                                code = int(action[2],base=0)
                              except Exception as e:
                                code = int(self.c_dict[cmds[i+1]][cmds[i+2]],base=0)
                              # end try
                              actions += struct.pack("<BI",
                                                     api.values["COMMANDS"]["INFRA_RED"][action[1]],
                                                     code)
                          elif action[0] == "DIRECTORY":
                              actions += bytes([api.values["COMMANDS"]["DIRECTORY"]])
                          else:
                              print("Unhandled %s" % (str(item[i])))
                              raise toto
                          # end if
                        # end for
                        i += 1
                    # end while
                    actions += bytes([api.values["COMMANDS"]["END"]])
                    # print("%4d %s" % (action_start,item[0]))
                    menu_actions.append({"label":string_index,
                                         "actions" : menu_choice,
                                         "start" : action_start})
                    # Space for END
                    menu_actions_length += 1
                # end for
                menu_numbers.update({str(m):len(self.menus)})
                self.menus.update({str(m) : {"choices" : menu_actions}})
            # end if
        # end while
        
        # Sort the strings 
        self.strings_ordered = dict()
        for s in self.strings_to_index.keys():
            self.strings_ordered.update({self.strings_to_index[s]:{"label":bytes(s,"utf-8")}})
        # end for

        # Compute the start of each
        start = 0
        self.strings_number = len(self.strings_ordered)


        self.strings_content = bytes()
        for i in range(self.strings_number):
            self.strings_ordered[i].update({"start":start})
            start += len(self.strings_ordered[i]["label"])+1
            self.strings_content += self.strings_ordered[i]["label"] + bytes([0x00])
        # end for



        # Reconsider menu : complete menu data structure
        self.menus_content = bytes()
        self.starting_menu_offset = -1
        self.menu_offsets = []
        
        for content in self.menus.keys():
            if content == "START":
                self.starting_menu_offset = len(self.menus_content)
            # end if
            self.menu_offsets.append(len(self.menus_content))
            choices = self.menus[content]["choices"]
            menu_content = struct.pack("B",len(choices))

            for choice in choices:
                menu_content += struct.pack("<BH",choice["label"],choice["start"])
            # end for
            
            self.menus[content].update({"start" : len(self.menus_content)})
            self.menus_content += menu_content
        # end for

        
        # Reconsider menu : complete action data structure
        self.actions_content = actions
        
        self.generated = True
    
        #print(binascii.hexlify(self.menus_content))
        #print(str(self.strings_content))
        #print(binascii.hexlify(actions))


    def dump(self):
        if not self.generated:
            self.generate()
        # end if

        WORDS_NB = len(self.words)
        
        cfg_data = bytes()

        header_format = "<HHHHHHHBBHHH"

        key_offset = struct.calcsize(header_format) +\
            2*self.strings_number +\
            len(self.strings_content) +\
            len(self.actions_content) +\
            len(self.menus_content)

        if key_offset % 16 != 0:
            key_offset += (16 - (key_offset%16))
        # endif

        word_length_offset = key_offset + len(self.picovoice_key)

        directory_offset = word_length_offset +\
            WORDS_NB * struct.calcsize("<f") +\
            WORDS_NB * struct.calcsize("<I") +\
            WORDS_NB * struct.calcsize("<I")
        
        for i in range(WORDS_NB):
            directory_offset += len(self.words[i]["data"])
        # end for
                
        
        cfg_data += struct.pack(header_format,
                                self.strings_number,
                                2*self.strings_number,
                                len(self.strings_content),
                                len(self.actions_content),
                                len(self.menus_content),
                                self.starting_menu_offset,
                                WORDS_NB,
                                ord(self.picovoice_language[0]),
                                ord(self.picovoice_language[1]),
                                key_offset,
                                word_length_offset,
                                directory_offset
                                )
                                
        
        
        #print("INITIAL MENU")
        #print("%x" % (self.starting_menu_offset))

        for i in range(self.strings_number):
            cfg_data += struct.pack("<H",self.strings_ordered[i]["start"])
        # end for

        for i in range(len(self.strings_ordered)):
            cfg_data += self.strings_ordered[i]["label"] + bytes([0])
        # end for
        
        #print("Strings [%d]" % (len(self.strings_ordered)))
        #for s in range(len(self.strings_ordered)):
        #    print(" %3d : %3d" % (s,self.strings_ordered[s]["start"]))
        # end for
        #for s in range(len(self.strings_ordered)):
        #    print(" %3d : %s" % (s,self.strings_ordered[s]["label"]))
        # end for

        cfg_data += self.actions_content

        offsets = b''
        for o in self.menu_offsets:
            offsets += struct.pack("<H",o)
        # end for
        # cfg_data += offsets
        
        cfg_data += self.menus_content

        #print("ACTIONS")
        #print(binascii.hexlify(self.actions_content))

        #print("MENU")
        #print(binascii.hexlify(self.menus_content))


        #print("PICOVOICE")
        if len(cfg_data) != key_offset:
            cfg_data += bytes([0] * (key_offset - len(cfg_data)))
        # end if

        cfg_data += self.picovoice_key.encode("utf-8")

        
        for i in range(WORDS_NB):
            cfg_data += struct.pack("<f",self.words[i]["threshold"])
        # end for
        for i in range(WORDS_NB):
            cfg_data += struct.pack("<I",self.words[i]["length"])
        # end for
        address = len(cfg_data) + 4 * WORDS_NB
        for i in range(WORDS_NB):
            cfg_data += struct.pack("<I",address)
            address += len(self.words[i]["data"])
        # end for
        for i in range(WORDS_NB):
            cfg_data += self.words[i]["data"]
        # end for


        for i in range(5):
            value = force_len(self.config_data["PHONE"][i][0],12)
            cfg_data += value
        # end for
        for i in range(5):
            for j in range(5):
                value = force_len(self.config_data["PHONE"][i][1][j][0],12)
                cfg_data += value
            # end for
        # end for
        for i in range(5):
            for j in range(5):
                for k in range(5):
                    value = force_len(self.config_data["PHONE"][i][1][j][1][k][0],12)
                    cfg_data += value
                    value = force_len(self.config_data["PHONE"][i][1][j][1][k][1],16)
                    cfg_data += value
            # end for
        # end for
        
        
        cfg = open("config.bin","wb")
        cfg.write(cfg_data)
        cfg.close()

    def status(self):
        print("%5s %4d %4d %4d %3d" % (str(self.veille),self.current_menu_offset,self.current_choice,self.current_first_choice,self.current_choice_level))

    def back(self):
        if not self.veille:
            if self.current_choice_level > 0:
                self.current_choice_level -= 1
                [self.current_menu_offset,
                 self.current_choice,
                 self.current_first_choice,
                 self.action_id] = self.choices[self.current_choice_level]
            else:
                self.veille = True
                self.action_id = 0
            # end if
        # end if

    def check_level(self):
        self.checker.current_choice = 0
        current_menu_length = self.menus_content[self.checker.current_menu]
        print("Menu length %d" % (current_menu_length))
        choice_offset = self.checker.current_menu + 1
        while self.checker.current_choice < current_menu_length:
            
            [string_id,action_offset] = struct.unpack("<BH",self.menus_content[choice_offset:choice_offset+3])
            while True:
                act = self.actions_content[action_offset]
                print("%4d %02x" % (action_offset,act))
                if act == api.values["ACTIONS"]["MENU"]:
                    menu_offset = struct.unpack("<H",self.actions_content[action_offset+1:action_offset+3])[0]
                    self.checker.push(menu_offset,string_id)
                    self.check_level()
                    self.checker.pop()
                    action_offset += 3
                elif act == api.values["ACTIONS"]["ACTION"]:
                    act_id = self.checker.get_action_id(string_id)
                    present = False
                    for cmd in self.commands_ordered:
                        if cmd[0] == act_id:
                            present = True
                            break
                        # end if
                    # end for
                    print("Looking for id %08x %s" % (act_id,str(present)))
                    if not present:
                        for cmd in self.commands_ordered:
                            if cmd[0] == act_id:
                                present = True
                                break
                            # end if
                        # end for
                        print("Second time %s" % str(present))
                        print(str(self.commands_ordered))
                        for i in range(self.checker.current_level):
                            choice_id = self.checker.choices[i][3]
                            print("%s" % (self.strings_ordered[choice_id]["label"]))
                        # end for
                        print("%s" % (self.strings_ordered[string_id]["label"]))
                        raise DataError("Not found")
                    action_offset += 1
                elif act == api.values["ACTIONS"]["DIRECTORY"]:
                    print("No check on PHONE")
                    action_offset += 1
                elif act in [api.values["ACTIONS"]["STAY"],
                             api.values["ACTIONS"]["SLEEP"],
                             api.values["ACTIONS"]["BACK"],
                             api.values["ACTIONS"]["MAIN"]]:
                    action_offset += 1
                elif act == api.values["ACTIONS"]["ACTION_n"]:
                    action_offset += 1
                elif act == api.values["ACTIONS"]["END"]:
                    break
                else:
                    raise toto
                # end if
            # end while
            choice_offset += 3
            self.checker.current_choice += 1
        # end while

    def generate_stm32_tests(self):
        f = open("ui_stm32.h","w")
        for k1 in self.c_dict["STM32"]["L3_AP"].keys():
            for k2 in self.c_dict["STM32"][k1].keys():
                if len(self.c_dict["STM32"][k1][k2]["input"]) != 0:
                    fields = []
                    f.write("field_t %s_%s [] = {\n" % (k1,k2))
                    for i in self.c_dict["STM32"][k1][k2]["input"]:
                        fields.append("  {\"%s\", TYPE_%s}" % (i[0],i[1]))
                    # end for
                    fields.append("  {NULL,0}")
                    f.write(",\n".join(fields))
                    f.write("};\n")
                # end if
            # end for
        # end for
        for k1 in self.c_dict["STM32"]["L3_AP"].keys():
            f.write("command_t command_%s[] = {\n" % k1)
            cmds = []
            for k2 in self.c_dict["STM32"][k1].keys():
                inputs = "NULL"
                if len(self.c_dict["STM32"][k1][k2]["input"]) != 0:
                    inputs = "%s_%s" % (k1,k2)
                #endif
                cmds.append("  {\"%s\",STM32__%s__%s,%s}" % (k2,k1,k2,inputs))
            # end for
            cmds.append("  {NULL,0}")
            f.write(",\n".join(cmds))
            f.write("\n};\n")
        # end for
        f.write("ap_t aps[] = {\n")
        aps = []
        for k in self.c_dict["STM32"]["L3_AP"].keys():
            aps.append("  {\"%s\", STM32__L3_AP__%s, command_%s, NULL}" % (k,k,k))
        # end for
        f.write(",\n".join(aps))
        f.write("\n};\n")
        f.close()
        
    def generate_bus_tests(self):
        f = open("ui_bus.h","w")
        f.write("bus_command_t commands[] = {\n")
        cmds = []
        for k in sorted(self.bus_tests.keys()):
            cmds.append("  {\"%s\",\"%s\"}"% (k,str(binascii.hexlify(self.bus_tests[k]))[2:-1]))
        # end for
        f.write(",\n".join(cmds))
        f.write("};\n")
        f.close()

    def generate_recursive_constant(self,result,base,tree):
        for k in tree.keys():
            if type(tree[k]) == type(0):
                txt = "#define %s%s %d\n" % (base,k,tree[k])
                result.append(txt)
            elif type(tree[k]) == type(""):
                # Try to convert as int
                try:
                    v = int(tree[k],0)
                    txt = "#define %s%s %s\n" % (base,k,tree[k])
                except Exception as e:
                    txt = "#define %s%s \"%s\"\n" % (base,k,tree[k])
                result.append(txt)
            else:
                if "command" in tree[k].keys():
                    txt = "#define %s%s %d\n" % (base,k,tree[k]["command"])
                    result.append(txt)
                else:
                    self.generate_recursive_constant(result,"%s%s__" % (base,k),tree[k])
                # end if
            # end if
        # end for
        
    def generate_constant(self):
        result = []
        interest = self.c_dict
        del(interest["picovoice"])
        self.generate_recursive_constant(result,"",self.c_dict)
        f = open("constant.h","w")
        for l in result:
            f.write(l)
        # end for
        f.close()
        
    def check(self):
        if not self.generated:
            self.generate()
        # end if
        self.checker = checker(self.strings_number)
        self.checker.current_menu = self.starting_menu_offset
        self.check_level()
        



name = "config.json"
if len(sys.argv) > 1:
    name = sys.argv[1]
# end if
f = open(name,"r")
js = json.loads(f.read())
f.close()

myGenerator = generator(js)
myGenerator.generate()
myGenerator.dump()
#myGenerator.check()
#myGenerator.generate_stm32_tests()
#myGenerator.generate_bus_tests()
#myGenerator.generate_constant()
