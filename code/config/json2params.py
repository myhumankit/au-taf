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
        self.base_address = 0x081C0000


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


        objs = [{"object":self.config_data["COMMANDS"],"name":""}]
        commands = dict()
        n = 0
        while len(objs) != 0:
            obj = objs.pop(0)
            
            if type(obj["object"]) == type(dict()):
                for k in obj["object"].keys():
                    objs.append({"object":obj["object"][k],
                                 "name":obj["name"] + "@" + k})
                # end for
            elif type(obj["object"]) == type([]):
                print("Adding %d %s" % (n,obj["name"]))
                commands.update({obj["name"]:{"rank":n,
                                              "actions":obj["object"]}})
                n += 1
            else:
                raise
            # end if
        # end while

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
                self.words = []
                for w in ["command", "next", "ok", "back"]:
                    if w not in self.config_data["CONFIG"]["picovoice"].keys():
                        raise;
                    # end if
                    name = self.config_data["CONFIG"]["picovoice"][w]["file"]
                    extension = name.split('.')[-1]
                    if extension == "h":
                        f = open(name,"rb")
                        data = htobin(f.read())
                        f.close()
                    elif extension == ".ppn":
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

                    self.words.append({"threshold" : float(self.config_data["CONFIG"]["picovoice"][w]["threshold"]),
                                       "data" : data,
                                       "length" : effective_length})
                # end for
            # end if
        # end if
        menus = ["START"]
        menus_done = []

        menu_numbers = dict()
        self.menus = dict()

        action_length = 0
        while len(menus) != 0:
            m = menus.pop(0)
            if m not in menus_done:
                menu_actions = []
                menu_actions_length = 0
                menus_done.append(m)
                for item in self.config_data["MENUS"][m]:
                    action_start = action_length
                    menu_choice = []
                    string_index = self.add_string(item[0])
                    i = 1
                    while i < len(item):
                        print(" Treating %s" % item[i])
                        if item[i] == "MENU":
                            submenu = item[i+1]
                            menu_choice.append(["MENU",submenu])
                            menus.append(submenu)
                            i += 2
                            action_length += 3
                        elif item[i] in ["ACTION",
                                         "ACTION_n",
                                         "ACTION_nn",
                                         "DIRECTORY",
                                         "STAY",
                                         "SLEEP",
                                         "BACK",
                                         "MAIN"]:
                            menu_choice.append([item[i]])
                            i += 1
                            action_length += 1
                        else:
                            print("Unhandled %s" % (str(item[i])))
                            raise toto
                        # end if
                    # end while
                    action_length += 1 # To end the stream
                    print("%4d %s" % (action_start,item[0]))
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

        # Assign an id to each command corresponding to the level
        # and also an index
        self.commands_content = bytes()
        self.commands = []
        self.bus_tests = dict()
        for name in commands.keys():
            print("%3d %4d Name %s" % (len(self.commands),
                                       len(self.commands_content),
                                       name))
            v = self.compute_id_from_choices(name)
            commands[name].update({"id":v,"index":len(self.commands)})
            self.commands.append({"detail":commands[name]["actions"],
                                  "start":len(self.commands_content)})
            cmds = commands[name]["actions"]
            i = 0
            while i < len(cmds):
                if cmds[i] == "BUS":
                    print("%d %s" % (i,cmds))
                    cmd = struct.pack("BBB",                    
                                      self.c_dict["COMMAND"]["BUS"],
                                      self.c_dict["BUS"][cmds[i+1]],
                                      self.c_dict["ADDRESS"][cmds[i+2]])
                    if cmds[i+1] == "BIT_SPECIAL":
                        cmd += struct.pack("B",
                                           int(cmds[i+3],base=0))
                    else :
                        cmd += struct.pack("BB",
                                           self.c_dict["PORT"][cmds[i+3]],
                                           int(cmds[i+4],base=0))
                    # end if
                    self.bus_tests.update({name:cmd})
                    self.commands_content += cmd
                    i += len(cmd)
                elif cmds[i] == "INFRA_RED":
                    try:
                        code = int(cmds[i+2],base=0)
                    except Exception as e:
                        code = int(self.c_dict[cmds[i+1]][cmds[i+2]],base=0)
                    # end try
                    self.commands_content += struct.pack("<BI",
                                                         api.values["COMMANDS"]["INFRA_RED"][cmds[i+1]],
                                                         code)
                    i += 5
                elif cmds[i] == "RF_433":
                    try:
                        code = int(cmds[i+2],base=0)
                    except Exception as e:
                        code = int(self.c_dict[cmds[i+1]][cmds[i+2]],base=0)
                    # end try
                    self.commands_content += struct.pack("<BI",
                                                         api.values["COMMANDS"]["RF_433"][cmds[i+1]],
                                                         code)
                    i += 5
                elif cmds[i] == "PHONE":
                    self.commands_content += bytes([api.values["COMMANDS"]["PHONE"][cmds[i+1]]])
                    if cmds[i+1] == "SMS":
                        self.commands_content += bytes([len(cmds[i+2])])
                        self.commands_content += cmds[i+2].encode("utf-8")
                        self.commands_content += bytes([len(cmds[i+3])])
                        self.commands_content += cmds[i+3].encode("utf-8")
                        i += 2
                    elif cmds[i+1] == "CALL":
                        self.commands_content += bytes([len(cmds[i+2])])
                        self.commands_content += cmds[i+2].encode("utf-8")
                        i += 1
                    i += 2                 
                else:
                    print(name)
                    print("%d %s" % (i,cmds))
                    raise
                # end if
            # end while
            self.commands_content += bytes([api.values["ACTIONS"]["END"]])
        # end for
        print("COMMANDS = %s" % (str(commands)))

        # Handle aliases
        
        for alias in self.config_data["ALIAS"]:
            ref = alias.pop(0)
            ref_key = "@" + "@".join(ref)
            print("REF %s" % ref_key)
            if ref_key not in commands.keys():
                print("NOT DEFINED")
            else:
                ref_index = commands[ref_key]["index"]
                for al in alias:
                    al_key = "@" + "@".join(al)
                    if al_key in commands.keys():
                        print("%s ALREADY DEFINED/ALIAS" % al_key)
                        raise
                    # end if
                    al_id = self.compute_id_from_choices(al_key)
                    commands.update({al_key:{"id":al_id,"index":ref_index}})
                # end for
            # end if
        # end for


        commands_unordered = dict()
        for name in commands.keys():
            commands_unordered.update({commands[name]["id"]:commands[name]["index"]})
        # end for

        self.commands_ordered = []
        for id in sorted(commands_unordered.keys()):
            self.commands_ordered.append([id,commands_unordered[id]])
        # end if


        # Reconsider menu : complete menu data structure
        self.menus_content = bytes()
        self.starting_menu_offset = -1
        
        for content in self.menus.keys():
            if content == "START":
                self.starting_menu_offset = len(self.menus_content)
            # end if
            choices = self.menus[content]["choices"]
            menu_content = struct.pack("B",len(choices))

            for choice in choices:
                menu_content += struct.pack("<BH",choice["label"],choice["start"])
            # end for
            
            self.menus[content].update({"start" : len(self.menus_content)})
            self.menus_content += menu_content
        # end for

        # Reconsider menu : complete action data structure
        self.actions_content = bytes()
        for full_content in menus_done:
            content = str(full_content)
            choices = self.menus[content]["choices"]

            actions_start = []
            choice_content = bytes()
            for choice in choices:
                for action in choice["actions"]:                    
                    print(" Treating %s" % action[0])
                    if action[0] == "MENU":
                        print(str(action))
                        print(str(action[1]))
                        print(self.menus[str(action[1])])
                        print("FINAL " + str(self.menus[str(action[1])]["start"]))
                        submenu_start = self.menus[str(action[1])]["start"]
                        choice_content += struct.pack("<BH",api.values["ACTIONS"]["MENU"],submenu_start)
                    elif action[0] in api.values["ACTIONS"].keys():
                        choice_content += bytes([api.values["ACTIONS"][action[0]]])
                    else:
                        raise toto
                    # end if
                # end for
                choice_content += bytes([api.values["ACTIONS"]["END"]])
            # end for
            self.actions_content += choice_content
        # end for
        
        self.generated = True

    def dump(self):
        if not self.generated:
            self.generate()
        # end if

        WORDS_NB = 4
        
        cfg_data = bytes()

        header_format = "<HHHHHHHHHHBBHHH"

        key_offset = struct.calcsize(header_format) +\
            2*self.strings_number +\
            len(self.strings_content) +\
            6*len(self.commands_ordered) +\
            len(self.commands_content)+\
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
                                len(self.commands_ordered),
                                6*len(self.commands_ordered),
                                len(self.commands_content),
                                len(self.actions_content),
                                len(self.menus_content),
                                self.starting_menu_offset,
                                WORDS_NB,
                                ord(self.picovoice_language[0]),
                                ord(self.picovoice_language[1]),
                                key_offset,
                                word_length_offset, #self.words_offset
                                directory_offset
                                )
                                
        
        
        print("INITIAL MENU")
        print("%x" % (self.starting_menu_offset))

        for i in range(self.strings_number):
            cfg_data += struct.pack("<H",self.strings_ordered[i]["start"])
        # end for

        for i in range(len(self.strings_ordered)):
            cfg_data += self.strings_ordered[i]["label"] + bytes([0])
        # end for
        
        print("Strings [%d]" % (len(self.strings_ordered)))
        for s in range(len(self.strings_ordered)):
            print(" %3d : %3d" % (s,self.strings_ordered[s]["start"]))
        # end for
        for s in range(len(self.strings_ordered)):
            print(" %3d : %s" % (s,self.strings_ordered[s]["label"]))
        # end for

        for cmd in self.commands_ordered:
            cfg_data += struct.pack("<IH",cmd[0],self.commands[cmd[1]]["start"])
        # end for

        cfg_data += self.commands_content

        print(binascii.hexlify(self.commands_content))
        
        print("ACTION")
        for i in range(len(self.commands)):
            print("%3d %4d %s" % (i,
                                  self.commands[i]["start"],
                                  self.commands[i]["detail"]))
        # end for

        print("COMMANDS")
        for cmd in self.commands_ordered:
            print("%08x %3d" % (cmd[0],self.commands[cmd[1]]["start"]))
        # end for

        cfg_data += self.actions_content
        
        print("ACTIONS")
        print(binascii.hexlify(self.actions_content))

        cfg_data += self.menus_content

        print("MENU")
        print(binascii.hexlify(self.menus_content))


        print("PICOVOICE")
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
        address = len(cfg_data) + 4 * 4 + self.base_address
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
        


f = open("config.json","r")
js = json.loads(f.read())
f.close()

myGenerator = generator(js)
myGenerator.generate()
myGenerator.dump()
myGenerator.check()
myGenerator.generate_stm32_tests()
myGenerator.generate_bus_tests()
#myGenerator.generate_constant()
