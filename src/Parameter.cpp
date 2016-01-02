#include "Parameter.h"
//#include "ConfigFile.h"
#include "Utility.h"

CParameter::CParameter(int _argc, char* _argv[]) {
    Clear();
    ParseCommandLine(_argc, _argv);
}

CParameter::~CParameter() {
    Clear();
}



void CParameter::CheckParams() {

}

void CParameter::PrintLine() {
    printf("***************************************\n");
}
void CParameter::PrintParams() {

    PrintLine();

    const char* workMode[] = {"learn svm model", "run Training App"};
    const char* verboseMode[] = {"no","yes"};

    int mode = m_workMode == WORKMODE_UI ? 1 : 0;
    printf("Mode          = %s\n", workMode[mode]);
    mode = m_isVerbose ? 1 : 0;
    printf("Verbose       = %s\n", verboseMode[mode]);

    printf("DataFile      = %s\n", m_dataDir.c_str());
    printf("ModelFile     = %s\n", m_modelFile.c_str());
    printf("ResultFile    = %s\n", m_saveDir.c_str());
    PrintLine();
}

void CParameter::PrintHelp() {
    printf("\nInvalid arguments!!!\n\n");

    printf("Quick Examples\n");
    printf("==============\n");

    printf("An example for learning attractive/serious svm models:\n");
    printf("./MirrorMirror -m 0 ./data/test/ ./data/models/\n\n");

    printf("An example for Expression Training App using trained model and video file:\n");
    printf("./MirrorMirror -m 1 -i ./data/video.avi ./data/target.png ./data/models/\n\n");

    printf("An example for Expression Training App using cross-subject model and webcam:\n");
    printf("./MirrorMirror -m 1 your/target/image/ ./models/svm/\n\n");

    printf("Command Usage ([]: options)\n");
    printf("===========================\n");
    printf("./MirrorMirror [-v verbose] [-m mode] [-i input_video] [-s  save_dir] data_file model_dir\n\n");
    printf("\n-v verbose: shows details about the runtime output (default = 1)\n");


    printf("-m mode: set the training mode (default=0)\n");
    printf("\t0 -- learning attractive/serious svm models\n");
    printf("\t1 -- expression training App\n");

    printf("-i input_video: specify the video input; if no video is specified, the system will use webcam.\n\n");
    printf("-s save_dir: specify the directory where the results will be saved.\n");

    printf("data_file: set the directory/path of the input data: if m = 0, data_file specifies the directory of images and scores used for svm training; if m = 1, data_file is the path of the selected target expression.\n\n");
    printf("model_dir: set the directory for svm models. (Cross-subject svm models trained on 11 subjects are included: ./models/svm)\n\n");
    exit(-1);
}

void CParameter::ParseCommandLine(int _argc, char *_argv[]) {
    int i = 0;
    int mode = 0;

    for (i = 1; i < _argc;i++) {
        if(_argv[i][0] != '-')
            break;
        if(++i >= _argc)
            PrintHelp();

        switch (_argv[i-1][1]) {
        case 'v':   // 1: verbose
            mode = atoi(_argv[i]);
            m_isVerbose = (mode == 1);
            break;
        case 'm':  // train mode: 0: training; 1: test
            mode = atoi(_argv[i]);
            m_workMode = mode == 0 ? WORKMODE_TRAIN : WORKMODE_UI;
            break;
        case 'i':
            m_videoFile = string(_argv[i]);
            break;
        case 's':
            m_saveDir = string(_argv[i]);
        case 'h':
            PrintHelp();
            break;
        default:
            PrintHelp();
            DEBUG_ERROR("Unknown option: -%c", _argv[i-1][1]);
        }
    }

    if (i >= _argc)
        PrintHelp();

    m_dataDir = string(_argv[i]);
    i++;

    if (i >= _argc)
        PrintHelp();
    m_modelFile = string(_argv[i]);

    if (m_saveDir.empty()) {
        unsigned found = m_dataDir.find_last_of(".");
        m_saveDir = m_dataDir.substr(0, found);
    }

    CheckParams();
    PrintParams();
}

void CParameter::Clear() {
    m_maxNumFrames = 10000;
    m_waitTime = 0;
    m_numFrameAlign = 5;
    m_numFrameH = 1;
    m_isVerbose = false;
    m_ratios.assign(4, 1.75);
    m_videoFile = "";
}
