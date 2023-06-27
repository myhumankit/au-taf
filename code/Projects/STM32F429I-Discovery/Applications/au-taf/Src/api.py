#! /usr/bin/env python3

API = {"COMMANDS" : {"RF_433" : ["PEREL"],
                     "INFRA_RED" : ["PHILIPS",
                                    "AKAI",
                                    "RC5",
                                    "AAA"],
                     "PHONE" : ["DIAL",
                                "VOL_H",
                                "VOL_L",
                                "SMS",
                                "HANGUP",
                                "UNHOOK"],
                      "MENU" : None,
                      "DIRECTORY" : None,
                      "END" : None,}}

values = dict()

def parse(prefix,obj,n,fct,user):
    if type(obj) == type(dict()):
        for k in obj.keys():
            n = parse(prefix + [k],obj[k],n,fct,user)
        #end for
    elif type(obj) == type([]):
        for v in obj:
            fct(prefix + [v],n,user)
            n += 1
        # end for
    elif obj is None:
        fct(prefix,n,user)
        n += 1
    else:
        fct(prefix + [obj],n,user)
        n += 1
    # end if
    return n

def dump(indexes,value,user):
    user.write("#define %s %d\n" % ("__".join(indexes),value))
    
def set_recursive(indexes, value,var):
    if type(indexes) == type([]):
        if len(indexes) > 1:
            if indexes[0] not in var.keys():
                var[indexes[0]] = dict()
            # end if
            set_recursive(indexes[1:],value,var[indexes[0]])
        else:
            var[indexes[0]] = value
        # end if
    else:
        var[indexes] = value
    # end if

    
if __name__ == "__main__":
    f = open("../Inc/constant.h","w")
    for top in ["COMMANDS"]:
        parse([top],API[top],0,dump,f)
    # end for
    f.close()
else:
    for top in ["COMMANDS"]:
        parse([top],API[top],0,set_recursive,values)
    # end for
# end if
