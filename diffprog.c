#include "diffprog.h"

int main() {
    START_LOGG;

    LOGG("<pre>\n");
    LOGG("<head><style>table {border-collapse: collapse;background-color: #e6f2ff;font-family: Arial, sans-serif;} th, td {border: 1px solid #b3d7ff;padding: 12px; text-align: center;}th {background-color: #cbdff7; font-weight: bold;}</style></head>\n");
    if (CreateDir(DIR_NAME) == -1) {
        abort();
    }

    struct bin_tree tree = {};
    InitTree(&tree);
    DownloadTree(&tree, SAVE_FILENAME);
    TreeDump(&tree, "dump", NO_VIEW);
    PrintTreePreorder(&tree, stdout);

    WriteDerivativeReportLatex(&tree, 'x', "report.tex");
    CompileLatexToPdf("report.tex");

    //PrintTreePreorder(&tree, stdout);
    SaveTree(&tree);
    TreeDump(&tree, "dump", NO_VIEW);

    STOP_LOGG;
    DestroyTree(&tree);
    printf("ok");
    return 0;
}
