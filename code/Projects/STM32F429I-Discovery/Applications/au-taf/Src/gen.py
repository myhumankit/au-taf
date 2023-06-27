#! /usr/bin/env python3

API = {"COMMANDS" : {"RF_433" : ["PEREL"],
                     "INFRA_RED" : ["PHILIPS",
                                    "AKAI",
                                    "RC5",
                                    "AAA"],
                     "PHONE" : ["DIAL",
                                "HANG",
                                "HOOK"]},
       "ACTIONS" : ["END",
                    "MENU",
                    "ACTION",
                    "STAY",
                    "BACK"]}

def dump(f,prefix,obj,n = 0):
    if type(obj) == type(dict()):
        for k in obj.keys():
            n = dump(f,prefix + k + "__",obj[k],n)
        #end for
    elif type(obj) == type([]):
        for v in obj:
            f.write("#define %s%s %d\n" % (prefix,v,n))
            n += 1
        # end for
    else:
        f.write("#define %s%s %d\n" % (prefix,obj,n))
        n += 1
    # end if
    return n

if __name__ == "__main__":
    f = open("constant.h","w")
    dump(f,"",API)
    f.close()
# end if
