#ifndef IPCMESSAGEJS_H
#define IPCMESSAGEJS_H

enum class IPCJSMsg {
    INITFRAMEWORK,
    EXECJSSERVICE,
    TAKEHEAPSNAPSHOT,
    EXECJS,
	CREATEINSTANCE,
    DESTORYINSTANCE,
    EXECJSONINSTANCE,
    EXECJSWITHRESULT,
    UPDATEGLOBALCONFIG,
    EXECTIMERCALLBACK,
};

enum class IPCProxyMsg {
    SETJSFVERSION,
    REPORTEXCEPTION,
    CALLNATIVE,
    CALLNATIVEMODULE,
    CALLNATIVECOMPONENT,
    CALLADDELEMENT,
    SETTIMEOUT,
    NATIVELOG,
    CALLCREATEBODY,
    CALLUPDATEFINISH,
    CALLCREATEFINISH,
    CALLREFRESHFINISH,
    CALLUPDATEATTRS,
    CALLUPDATESTYLE,
    CALLREMOVEELEMENT,
    CALLMOVEELEMENT,
    CALLADDEVENT,
    CALLREMOVEEVENT,
    CALLGCANVASLINK,
	CALLT3DLINK,
    SETINTERVAL,
    CLEARINTERVAL,
};
#endif /* IPCMESSAGEJS_H */
