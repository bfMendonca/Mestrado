unit internal;

interface
uses messages,windows,math,sysutils;

const WM_IPCTHREADACCESS=WM_USER+130;
      LIB_VERSION=1200;
      MAX_SIZE=$7F00;

      FSUIPC_ERR_OK	           =  0;
      FSUIPC_ERR_OPEN          =  1;  // Attempt to Open when already Open
      FSUIPC_ERR_NOFS          =  2;  // Cannot link to FSUIPC or WideClient
      FSUIPC_ERR_VIEW          =  6;  // Failed to open a view to the file map
      FSUIPC_ERR_WRONGFS       =  8;  // Sim is not version requested
      FSUIPC_ERR_NOTOPEN       =  9;  // Call cannot execute, link not Open
      FSUIPC_ERR_NODATA        = 10;  // Call cannot execute: no requests accumulated
      FSUIPC_ERR_DATA          = 13;  // IPC request contains bad data
      FSUIPC_ERR_SIZE          = 15;  // Read or Write request cannot be added, memory for Process is full

      FS6IPC_MESSAGE_SUCCESS   = 1;
      // IPC message types
      FS6IPC_READSTATEDATA_ID  = 1;
      FS6IPC_WRITESTATEDATA_ID = 2;
      //Misc
      SIM_FS2K4	           =  7;


type pword= ^word;
     pbyte= ^byte;

     FS6IPC_READSTATEDATA_HDR=record
       dwId     : dword;
       dwOffset : dword;
       nBytes   : dword;
       pDest    : pointer;
       end;

     FS6IPC_WRITESTATEDATA_HDR=record
       dwId     : dword;
       dwOffset : dword;
       nBytes   : dword;
       end;


var FSUIPC_VERSION:word;
    FSUIPC_FS_Version:word;
    FSUIPC_Lib_Version:word;

    m_hwnd:hwnd=0;
    m_pview:pointer;
    m_pnext:pointer;
    m_ulMax:longword;

procedure FSUIPC_Close;
function  FSUIPC_Open2(dwFSReq:word;var dwresult:Dword; pMem: pbyte; dwsize:word ):boolean;
function  FSUIPC_Process(var dwresult:dword):boolean;
Function  FSUIPC_ReadCommon(fspecial:boolean;dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
Function  FSUIPC_Read(dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
Function  FSUIPC_ReadSpecial(dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
Function  FSUIPC_Write(dwOffset : DWORD; dwSize : DWORD; pSrce : Pointer; var dwResult : DWORD) : Boolean;


implementation

procedure FSUIPC_Close;
begin
m_hWnd:=0;
m_pView:=nil;
end;

function FSUIPC_open2(dwFSReq:word;var dwresult:Dword; pMem: pbyte; dwsize:word ):boolean;
begin
// abort if already started
if m_pview<>nil then
   begin
   dwresult:=FSUIPC_ERR_OPEN;
   FSUIPC_open2:=false;
   exit;
   end;

if (pMem=nil) or (dwsize<32) then
   begin
   dwresult:=FSUIPC_ERR_VIEW;
   FSUIPC_open2:=false;
   exit;
   end;

// Clear version information, so know when connected
FSUIPC_FS_Version:=0;
FSUIPC_Version:=FSUIPC_FS_Version;

// Connect via FSUIPC, which is known to be FSUIPC's own
// and isn't subject to user modificiation
m_hWnd := FindWindowEx(0, 0, PChar('UIPCMAIN'), Nil);
if m_hWnd=0 then
   begin
   dwResult:=FSUIPC_ERR_NOFS;
   FSUIPC_open2:=false;
   exit;
   end;

// get an area of memory
m_ulMax:=min(dwSize,MAX_SIZE);
m_pView:=pMem;

// Okay, now determine FSUIPC version AND FS type
DWORD(m_pNext):=DWORD(m_pView) + 4; // Allow space for code pointer

// Read FSUIPC version
if FSUIPC_Read($3304, 4, @FSUIPC_Version, dwResult)=false then
   begin
   FSUIPC_Close;
   FSUIPC_open2:=false;
   exit;
   end;

// and FS version and validity check pattern
if FSUIPC_Read($3308, 4, @FSUIPC_FS_Version, dwResult)=false then
   begin
   FSUIPC_Close;
   FSUIPC_open2:=false;
   exit;
   end;

// Write our Library version number to a special read-only offset
// This is to assist diagnosis from FSUIPC logging

if FSUIPC_Write($330a, 2, @FSUIPC_Lib_Version, dwResult)=false then
   begin
   FSUIPC_Close;
   FSUIPC_open2:=false;
   exit;
   end;

// Actually send the requests and get the responses ("process")
if FSUIPC_Process(dwResult)=false then
   begin
   FSUIPC_Close();
   FSUIPC_open2:=false;
   exit;
   end;



// Only allow running on FSUIPC 1.998e or later
// with correct check pattern 0xFADE
{if ((FSUIPC_Version < $19980005) or ((FSUIPC_FS_Version and $FFFF0000) <> ($FADE0000))) then
   begin
   dwResult:=FSUIPC_ERR_VERSION;
   FSUIPC_Close;
   FSUIPC_Open2:=false;
   end;

FSUIPC_FS_Version := (FSUIPC_FS_Version and $ffff); // Isolates the FS version number
if (dwFSReq<>0) and  (dwFSReq <> FSUIPC_FS_Version) then // Optional user specific FS request
   begin
   dwResult:=FSUIPC_ERR_WRONGFS;
   FSUIPC_Close;
   FSUIPC_Open2:=false;
   end;
              }
dwresult:=FSUIPC_ERR_OK;
FSUIPC_open2:=true;
end;



function fsuipc_process(var dwresult:dword):boolean;
var
  dwError : DWORD;
  pdw     : ^DWORD;  //Pointer;
  pHdrR   : ^FS6IPC_READSTATEDATA_HDR;
  pHdrW   : ^FS6IPC_WRITESTATEDATA_HDR;
  pTemp:^DWORD;

begin

if (m_pView = Nil) then
   begin
    dwResult := FSUIPC_ERR_NOTOPEN;
    Result   := FALSE;
    Exit;
  end;

if (DWORD(m_pView)+4 >= DWORD(m_pNext)) then
  begin
    dwResult := FSUIPC_ERR_NODATA;
    Result   := FALSE;
    Exit;
  end;

ZeroMemory(m_pNext, 4); // Terminator

// send the request
asm
 push eax
 call @next
 @next: pop eax
 mov dwError,eax
 pop eax
 end;

//ptemp:=m_pview;
//dword(ptemp):=dwerror;

dwError:= sendmessage(m_hwnd,WM_IPCTHREADACCESS,dword(m_pNext)-dword(m_pView)-4,dword(m_pView) );

dword(m_pnext):=dword(m_pView)+4;

if dwError<>FS6IPC_MESSAGE_SUCCESS then
   begin
   dwResult:=FSUIPC_ERR_DATA;
   Result   := FALSE;
   Exit;
   end;

dword(pdw):=dword(m_pview)+4;

while (pdw <> Nil) and Boolean(pdw^) do
  begin
  case pdw^ of
    FS6IPC_READSTATEDATA_ID :
      begin
      pHdrR := Pointer(pdw);
      DWORD(m_pNext) := DWORD(m_pNext) + sizeof(FS6IPC_READSTATEDATA_HDR);
      if (pHdrR^.pDest <> Nil) and (pHdrR^.nBytes <> 0) then
         begin
         CopyMemory(pHdrR^.pDest, m_pNext, pHdrR^.nBytes);
         end;
      DWORD(m_pNext) := DWORD(m_pNext) + pHdrR^.nBytes;
      end;
    FS6IPC_WRITESTATEDATA_ID :
      begin
      // This is a write, so there's no returned data to store
      pHdrW := Pointer(pdw);
      DWORD(m_pNext) := DWORD(m_pNext) + sizeof(FS6IPC_WRITESTATEDATA_HDR) + pHdrW^.nBytes;
      end;
    else
    // Error! So terminate the scan
    pdw := Nil;
    end;
  dword(pdw):=dword(m_pNext);
  end;

dword(m_pNext)  := dword(m_pView)+4;
dwResult := FSUIPC_ERR_OK;
Result   := TRUE;
end;


Function  FSUIPC_ReadCommon(fspecial:boolean;dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
var
  pHdr : ^FS6IPC_READSTATEDATA_HDR;

begin
dword(pHdr) := dword(m_pNext);

// Check link is open
if (m_pView = Nil) then
    begin
    dwResult := FSUIPC_ERR_NOTOPEN;
    Result   := FALSE;
    Exit;
    end;

// Check have space for this request (including terminator)
if (((DWORD(m_pNext) - DWORD(m_pView)) + 8 + (dwSize + sizeof(FS6IPC_READSTATEDATA_HDR))) > m_ulMax) then
    begin
    dwResult := FSUIPC_ERR_SIZE;
    Result   := FALSE;
    Exit;
    end;

// Initialise header for read request
pHdr^.dwId     := FS6IPC_READSTATEDATA_ID;
pHdr^.dwOffset := dwOffset;
pHdr^.nBytes   := dwSize;
pHdr^.pDest    := pDest;

//update pointer
DWORD(m_pNext) := DWORD(m_pNext) + sizeof(FS6IPC_READSTATEDATA_HDR);

// Initialise the reception area, so rubbish won't be returned
if (dwSize <> 0) then
   begin
   if fspecial=true
       then CopyMemory(m_pNext,pdest,dwsize)
       else ZeroMemory(m_pNext, dwSize);
   end;

// Update the pointer ready for more data
DWORD(m_pNext) := DWORD(m_pNext)+ dwsize;

dwResult := FSUIPC_ERR_OK;
Result   :=  TRUE;
end;


Function  FSUIPC_Read(dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
begin
FSUIPC_ReadCommon(false,dwoffset,dwsize,pDest,dwResult);
end;

Function  FSUIPC_ReadSpecial(dwOffset : DWORD; dwSize : DWORD; pDest : Pointer; var dwResult : DWORD) : Boolean;
begin
FSUIPC_ReadCommon(true,dwoffset,dwsize,pDest,dwResult);
end;

Function  FSUIPC_Write(dwOffset : DWORD; dwSize : DWORD; pSrce : Pointer; var dwResult : DWORD) : Boolean;
var
  pHdr : ^FS6IPC_WRITESTATEDATA_HDR;

begin
pHdr := m_pNext;

if (m_pView = Nil) then
   begin
   dwResult := FSUIPC_ERR_NOTOPEN;
   Result   := FALSE;
   Exit;
   end;

// Check have space for this request (including terminator)
if (((DWORD(m_pNext) - DWORD(m_pView)) + 8 +(dwSize + sizeof(FS6IPC_WRITESTATEDATA_HDR))) > m_ulMax) then
   begin
   dwResult := FSUIPC_ERR_SIZE;
   Result   := FALSE;
   Exit;
   end;

// Initialise header for write request
pHdr^.dwId     := FS6IPC_WRITESTATEDATA_ID;
pHdr^.dwOffset := dwOffset;
pHdr^.nBytes   := dwSize;

// Update the pointer ready for write
dword(m_pNext):=dword(m_pNext)+ sizeof(FS6IPC_WRITESTATEDATA_HDR);

// Copy in the data to be written
if (dwSize<>0)then CopyMemory(m_pNext, pSrce, dwSize);

// Update the pointer ready for more data
dword(m_pNext):=dword(m_pNext)+dwSize;

dwResult := FSUIPC_ERR_OK;
Result   := TRUE;
end;

Initialization

//--- Initialize global variables ---
FSUIPC_Version       := 0;
FSUIPC_FS_Version    := 0;
FSUIPC_Lib_Version   := LIB_VERSION;
//--- IPC Client Stuff ---
m_hWnd    := 0;      // FS6 window handle
m_pView   := Nil;    // pointer to view of file-mapping object
m_pNext   := Nil;

finalization
//--- Automatic "close" if programmer "forgets" ---
FSUIPC_Close;

end.
