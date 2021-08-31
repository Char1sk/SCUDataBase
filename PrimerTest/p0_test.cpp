#include <iostream>
#include <vector>
#include <memory>
#include "./p0_starter.h"

int main()
{
    std::cout << "Set Matrix 1" << std::endl;
    scudb::RowMatrix<int> rmat1(3,4);
    for (int r = 0; r < rmat1.GetRowCount(); ++r)
    {
        for (int c = 0; c < rmat1.GetColumnCount(); ++c)
        {
            rmat1.SetElement(r, c, r*rmat1.GetColumnCount()+c);
        }
    }
    scudb::RowMatrixOperations<int>::Print(rmat1);
    
    std::vector<int> vec34(3*4, 1);
    std::vector<int> vec33(3*3, 1);
    
    std::cout << "Filled Matrix 2" << std::endl;
    scudb::RowMatrix<int> rmat2(3,4);
    rmat2.FillFrom(vec34);
    scudb::RowMatrixOperations<int>::Print(rmat2);
    
    std::cout << "Filled Matrix 3" << std::endl;
    scudb::RowMatrix<int> rmat3(4,3);
    rmat3.FillFrom(vec34);
    scudb::RowMatrixOperations<int>::Print(rmat3);
    
    std::cout << "Filled Matrix 4" << std::endl;
    scudb::RowMatrix<int> rmat4(3,3);
    rmat4.FillFrom(vec33);
    scudb::RowMatrixOperations<int>::Print(rmat4);
    
    std::cout << "Added Matrix 5 from 1 and 2" << std::endl;
    std::unique_ptr<scudb::RowMatrix<int>> uprmat5 = scudb::RowMatrixOperations<int>::Add(&rmat1, &rmat2);
    scudb::RowMatrixOperations<int>::Print(*uprmat5);
    
    std::cout << "Multiplied Matrix 6 from 2 and 3" << std::endl;
    std::unique_ptr<scudb::RowMatrix<int>> uprmat6 = scudb::RowMatrixOperations<int>::Multiply(&rmat2, &rmat3);
    scudb::RowMatrixOperations<int>::Print(*uprmat6);
    
    std::cout << "GEMM Matrix 7 from 2, 3 and 4" << std::endl;
    std::unique_ptr<scudb::RowMatrix<int>> uprmat7 = scudb::RowMatrixOperations<int>::GEMM(&rmat2, &rmat3, &rmat4);
    scudb::RowMatrixOperations<int>::Print(*uprmat7);
    
    return 0;
}