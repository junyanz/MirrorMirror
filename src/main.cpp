#include "MirrorMain.h"
#include "Parameter.h"
#include "common.h"

int main(int _argc, char* _argv[]) {
    CParameter* param = new CParameter(_argc, _argv);
    CMirrorMain* mirror = new CMirrorMain(param);
    mirror->Run();
    DELETE_OBJECT(mirror);
    DELETE_OBJECT(param);

    return 0;
} 
