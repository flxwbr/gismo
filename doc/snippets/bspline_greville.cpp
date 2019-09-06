#include <gismo.h>

using namespace gismo;

int main(int, char**)
{
    gsInfo.precision(3);
    gsKnotVector<> kv (-1, 0, 3,3, 1 );
    gsBSplineBasis<> bsp(kv);

    gsMatrix<> greville  = bsp.anchors();
    gsInfo << greville <<  "\n";

    gsMatrix<> evaluate  = bsp.eval( greville );
    gsInfo << evaluate <<  "\n";

    return 0;
}
