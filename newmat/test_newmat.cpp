

#include "test_newmat.h"
#include <stdlib.h>
#include <limits>

void printMatrix(const std::string& s, Matrix& m) {
    std::cout << s  << std::endl;
    for (int r=0; r < m.Nrows(); ++r) {
        std::cout << "row " << r + 1 << ":\t";
        for (int c=0; c < m.Ncols(); ++c)
            std::cout << m.element(r,c) << "\t";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void testLongAlgorithm(int R, int C, int Max, int Reps) {
    printf("\n\nRunning testLongAlgorithm: Rows: %d, Cols: %d, Max: %d, Proj Reps: %d\n\n", R, C, Max, Reps);

    Matrix xg(R,C);
    int counter = 0;
    for (int r=0; r < xg.Nrows(); ++r)
        for (int c=0; c < xg.Ncols(); ++c) {
            if (c == 0)
                xg.element(r,c) = 0;
            else if (c == 1)
                xg.element(r,c) = 1;
            else {
                xg.element(r,c) = (double) rand() / ((double) UINT_MAX + 1) * 1000.0;
            }
        }
    //printMatrix("xg matrix", xg);

    ColumnVector wg(R);
    for (int r=0; r < wg.Nrows(); ++r)
        wg.element(r) = (double) rand() / ((double) UINT_MAX + 1) * 100.0;
    //printMatrix("wg vector (rate)", wg);

    ColumnVector deltag(R);
    for (int r=0; r < deltag.Nrows(); ++r)
        deltag.element(r) = 1.0/(double) rand();
    //printMatrix("deltag vector (weight)", deltag);

    ColumnVector reciprocal_deltag(R);
    for (int r=0; r < reciprocal_deltag.Nrows(); ++r)
        reciprocal_deltag.element(r) = 1/deltag.element(r);
    //printMatrix("reciprocal deltag vector", reciprocal_deltag);

    Matrix matrix_best(R,R/*???*/);
    for (int r=0; r < matrix_best.Nrows(); ++r)
        for (int c=0; c < matrix_best.Ncols(); ++c) {
            matrix_best.element(r,c) = -std::numeric_limits<double>::max();
        }
    //printMatrix("initial matrix_best matrix", matrix_best);

    //boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    for (int i=0; i < R; ++i) {         
        // reset all column elements of first column to zero
        for (int r=0; r < xg.Nrows(); ++r) {
            xg.element(r,0) = 0;
        }
        //printMatrix("xg matrix (reset)",xg);
        
        for (int j=0; j < Max; ++j) {
            // add neighbor case to collection
            int rand_neighbor = j;//Equilikely(0L,R-1,rng);
            xg.element(rand_neighbor,0) = 1;
            //printMatrix("xg matrix (adding neighbor)",xg);
            
            // calculate beta matrix
            //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation);
            //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part1);
            Matrix beta_part1 = (xg.t() * reciprocal_deltag.AsDiagonal() * xg);
            //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part1);

            //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part2);
            Matrix beta_part2 = (xg.t() * reciprocal_deltag.AsDiagonal() * wg);
            //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part2);
            //Matrix beta = beta_part1 * beta_part2;

            //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_combine);
            Matrix beta = beta_part1.i() * beta_part2;
            //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_combine);

            //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation);
            //printMatrix("beta matrix", beta);
            
            // calculate statistic
            matrix_best.element(i,j) = 0;
            for (int k=0; k < R; ++k) {    
                Real product = 0;
                for (int c=0; c < xg.Ncols(); ++c) // xg.Ncols() == beta.Nrows()
                    product += xg.element(k,c) * beta.element(c,0);
                double m = matrix_best.element(i,j) + deltag.element(k) * pow(wg.element(k) - product, 2);
                matrix_best.element(i,j) = m;
            }
            matrix_best.element(i,j) = -log(matrix_best.element(i,j));
        }
        //printMatrix("matrix_best matrix", matrix_best);
    }
    //printMatrix("final matrix_best matrix", matrix_best);
    //ReportTimeEstimate(StartTime, Reps, 1, &Console);
    //macroRunTimeManagerPrint(stdout);
}

void testOptimizedAlgorithm(int R, int C, int Max, int Reps) {
    printf("\n\nRunning testOptimizedAlgorithm: Rows: %d, Cols: %d, Max: %d, Proj Reps: %d\n\n", R, C, Max, Reps);

    //RandomNumberGenerator rng;

    //macroRunTimeManagerInit();

    Matrix xg(R,C);
    int counter = 0;
    for (int r=0; r < xg.Nrows(); ++r)
        for (int c=0; c < xg.Ncols(); ++c) {
            if (c == 0)
                xg.element(r,c) = 0;
            else if (c == 1)
                xg.element(r,c) = 1;
            else
                xg.element(r,c) = (double) rand() / ((double) UINT_MAX + 1) * 1000.0;
        }
    printMatrix("xg matrix", xg);

    ColumnVector wg(R);
    for (int r=0; r < wg.Nrows(); ++r)
        wg.element(r) = (double) rand() / ((double) UINT_MAX + 1) * 100.0;
    printMatrix("wg vector (rate)", wg);

    ColumnVector deltag(R);
    for (int r=0; r < deltag.Nrows(); ++r)
        deltag.element(r) = 1.0/(double) rand();
    printMatrix("deltag vector (weight)", deltag);

    ColumnVector w_div_delta(R);
    for (int r=0; r < w_div_delta.Nrows(); ++r)
        w_div_delta.element(r) = wg.element(r)/deltag.element(r);
    printMatrix("w_div_delta vector (weight/delta)", w_div_delta);

    Matrix matrix_best(R,R/*???*/);
    for (int r=0; r < matrix_best.Nrows(); ++r)
        for (int c=0; c < matrix_best.Ncols(); ++c) {
            matrix_best.element(r,c) = -std::numeric_limits<double>::max();
        }
    printMatrix("initial matrix_best matrix", matrix_best);

    //boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    for (int i=0; i < R; ++i) {         
        // reset all column elements of first column to zero
        for (int r=0; r < xg.Nrows(); ++r) {
            xg.element(r,0) = 0;
        }
        printMatrix("xg matrix (reset)",xg);
        
        Matrix tobeinversed = xg;
        for (int r=0; r < tobeinversed.Nrows(); ++r)
            for (int c=0; c < tobeinversed.Ncols(); ++c) {
                tobeinversed.element(r,c) = tobeinversed.element(r,c)/deltag.element(r);
            }
        printMatrix("tobeinversed matrix", tobeinversed);
        Matrix t = xg.t();
        printMatrix("xg.t() matrix", t);
        tobeinversed = xg.t() * tobeinversed;
        printMatrix("xg.t() * tobeinversed matrix", tobeinversed);

        Matrix xgsigmaw = xg.t() * w_div_delta;
        //** printMatrix("xgsigmaw matrix", xgsigmaw);

        int mincluster=10;
        for (int j=0; j < Max; ++j) {
            // add neighbor case to collection
            int rand_neighbor = j;//Equilikely(0L,R-1,rng);
            xg.element(rand_neighbor,0) = 1;
            printMatrix("xg matrix (adding neighbor)",xg);

            tobeinversed.element(0,0) = tobeinversed.element(0,0) + (1/deltag.element(rand_neighbor));
            for (int k=1; k < tobeinversed.Ncols(); ++k) {
                tobeinversed.element(0,k) = tobeinversed.element(0,k) + xg.element(rand_neighbor,k)/deltag.element(rand_neighbor);
                tobeinversed.element(k,0) = tobeinversed.element(0,k);
            }
            printMatrix("tobeinversed matrix",tobeinversed);

            xgsigmaw.element(0,0) = xgsigmaw.element(0,0) + w_div_delta.element(rand_neighbor);

            if (j >= mincluster /* dist[i,j] < dist[i,j+1] */) {
                //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part1);

                //Matrix temp_matrix = tobeinversed.i() * (xg.t() * w_div_delta);
                Matrix temp_matrix = tobeinversed.i() * xgsigmaw;

                //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part1);
                if (temp_matrix.element(0,0) < 0) {
                    //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_combine);
                    temp_matrix = xg * temp_matrix;
                    //printMatrix("temp_matrix matrix", temp_matrix);
                    temp_matrix = wg - temp_matrix;
                    //printMatrix("temp_matrix matrix", temp_matrix);
                    temp_matrix = SP(temp_matrix, temp_matrix);
                    matrix_best.element(i,j) = -log((deltag.t() * temp_matrix).AsScalar());
                    //macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_combine);
                }
            }
        }
    }
    printMatrix("final matrix_best matrix", matrix_best);
    //ReportTimeEstimate(StartTime, Reps, 1, &Console);
    //macroRunTimeManagerPrint(stdout);
}

void runTest() {
    int R = 35, C = 4, Max=R/2, Reps=1000;
    testLongAlgorithm(R, C, Max, Reps);
    testOptimizedAlgorithm(R, C, Max, Reps);

    R = 500, Max=R/2;
    testLongAlgorithm(R, C, Max, Reps);
    testOptimizedAlgorithm(R, C, Max, Reps);

    R = 1000, Max=R/2;
    testLongAlgorithm(R, C, Max, Reps);
    testOptimizedAlgorithm(R, C, Max, Reps);    
    
    R = 3000, Max=R/2;
    testLongAlgorithm(R, C, Max, Reps);
    testOptimizedAlgorithm(R, C, 1000, Reps);    
}