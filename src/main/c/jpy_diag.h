#ifndef JPY_DIAG_H
#define JPY_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JPy_DiagFlags
{
    PyObject_HEAD
    int off;
    int type;
    int meth;
    int exec;
    int mem;
    int all;
}
JPy_DiagFlags;


#define JPy_DIAG_OFF    0x00
#define JPy_DIAG_TYPE   0x01
#define JPy_DIAG_METH   0x02
#define JPy_DIAG_EXEC   0x04
#define JPy_DIAG_MEM    0x08
#define JPy_DIAG_ALL    0xff

extern PyTypeObject DiagFlags_Type;
extern int JPy_ActiveDiagFlags;

PyObject* DiagFlags_New();

void JPy_DiagPrint(int diagFlags, const char * format, ...);

#define JPy_DIAG_PRINT if (JPy_ActiveDiagFlags != 0) JPy_DiagPrint


#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_DIAG_H */