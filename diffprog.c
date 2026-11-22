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

    struct bin_tree derivative = {};
    InitTree(&derivative);
    derivative.root = Diff(tree.root, 'x');
    TreeDump(&derivative, "raw derivative", NO_VIEW);

    struct bin_tree simplified = {};
    InitTree(&simplified);
    simplified.root = Simplify(derivative.root);
    TreeDump(&simplified, "simplified derivative", NO_VIEW);


    PrintTreePreorder(&tree, stdout);
    SaveTree(&tree);
    TreeDump(&tree, "dump", NO_VIEW);

    STOP_LOGG;
    DestroyTree(&tree);
    printf("ok");
    return 0;
}
