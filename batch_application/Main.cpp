//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "UtilityFunctions.h"
#include "Parameters.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "PrintScreen.h"
#include "AnalysisRun.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
#include "SSException.h"
#include "ParametersPrint.h"
#include "Toolkit.h"
#include "newmat.h"


/** Validates arguments of argument list. Throws UsageException if invalid.
    Returns whether options suppress execution of analysis. */
bool validateCommandLineArguments(int argc, char *argv[]) {
  if (argc < 2) throw usage_error(argv[0]);
  for (int i=2; i < argc; ++i) {
    if (!stricmp(argv[i], "-o")) {
      if (argc < i + 2) throw usage_error(argv[0]);
      ++i; //next parameter is assumed to be filename
      continue;
    }
    if (stricmp(argv[i], "-p") && stricmp(argv[i], "-c") &&
        stricmp(argv[i], "-one-cpu") && stricmp(argv[i], "-centric") && stricmp(argv[i], "-all-out"))
      throw usage_error(argv[0]);
  }
  for (int i=2; i < argc; ++i)
     if (!stricmp(argv[i], "-p") || !stricmp(argv[i], "-c")) return true;
  return false;
}

/** Returns index of 'arg' in argument list, returns zero if not found. */
int getCommandLineArgumentIndex(int argc, char *argv[], const char * arg) {
  for (int i=2; i < argc; ++i)
     if (!stricmp(argv[i], arg)) return i;
  return 0;
}

void __SaTScanInit(const char * sApplicationFullPath) {
  reserve_memory_cache();
  std::set_new_handler(prg_new_handler);
  AppToolkit::ToolKitCreate(sApplicationFullPath);
}

void __SaTScanExit() {
  release_memory_cache();
  AppToolkit::ToolKitDestroy();
}

void printMatrix(const std::string& s, Matrix& m) {
    /*std::cout << s  << std::endl;
    for (int r=0; r < m.Nrows(); ++r) {
        std::cout << "row " << r + 1 << ":\t";
        for (int c=0; c < m.Ncols(); ++c)
            std::cout << m.element(r,c) << "\t";
        std::cout << std::endl;
    }
    std::cout << std::endl;*/
}

void testLongAlgorithm(PrintScreen& Console, int R, int C, int Max, int Reps) {
    Console.Printf("\n\nRunning testLongAlgorithm: Rows: %d, Cols: %d, Max: %d, Proj Reps: %d\n\n", BasePrint::P_STDOUT, R, C, Max, Reps);

    RandomNumberGenerator rng;

    macroRunTimeManagerInit();

    Matrix xg(R,C);
    int counter = 0;
    for (int r=0; r < xg.Nrows(); ++r)
        for (int c=0; c < xg.Ncols(); ++c) {
            if (c == 0)
                xg.element(r,c) = 0;
            else if (c == 1)
                xg.element(r,c) = 1;
            else
                xg.element(r,c) = rng.GetRandomDouble() * 1000;
        }
    //printMatrix("xg matrix", xg);

    ColumnVector wg(R);
    for (int r=0; r < wg.Nrows(); ++r)
        wg.element(r) = rng.GetRandomDouble() * 100;
    //printMatrix("wg vector (rate)", wg);

    ColumnVector deltag(R);
    for (int r=0; r < deltag.Nrows(); ++r)
        deltag.element(r) = rng.GetRandomDouble();
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

    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    for (int i=0; i < R; ++i) {         
        // reset all column elements of first column to zero
        for (int r=0; r < xg.Nrows(); ++r) {
            xg.element(r,0) = 0;
        }
        //printMatrix("xg matrix (reset)",xg);
        
        for (int j=0; j < Max; ++j) {
            // add neighbor case to collection
            int rand_neighbor = Equilikely(0L,R-1,rng);
            xg.element(rand_neighbor,0) = 1;
            //printMatrix("xg matrix (adding neighbor)",xg);
            
            // calculate beta matrix
            //macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation);
            macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part1);
            Matrix beta_part1 = (xg.t() * reciprocal_deltag.AsDiagonal() * xg);
            macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part1);

            macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part2);
            Matrix beta_part2 = (xg.t() * reciprocal_deltag.AsDiagonal() * wg);
            macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part2);
            //Matrix beta = beta_part1 * beta_part2;

            macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_combine);
            Matrix beta = beta_part1.i() * beta_part2;
            macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_combine);

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
    ReportTimeEstimate(StartTime, Reps, 1, &Console);
    macroRunTimeManagerPrint(stdout);
}

void testOptimizedAlgorithm(PrintScreen& Console, int R, int C, int Max, int Reps) {
    Console.Printf("\n\nRunning testOptimizedAlgorithm: Rows: %d, Cols: %d, Max: %d, Proj Reps: %d\n\n", BasePrint::P_STDOUT, R, C, Max, Reps);

    RandomNumberGenerator rng;

    macroRunTimeManagerInit();

    Matrix xg(R,C);
    int counter = 0;
    for (int r=0; r < xg.Nrows(); ++r)
        for (int c=0; c < xg.Ncols(); ++c) {
            if (c == 0)
                xg.element(r,c) = 0;
            else if (c == 1)
                xg.element(r,c) = 1;
            else
                xg.element(r,c) = rng.GetRandomDouble() * 1000;
        }
    printMatrix("xg matrix", xg);

    ColumnVector wg(R);
    for (int r=0; r < wg.Nrows(); ++r)
        wg.element(r) = rng.GetRandomDouble() * 100;
    printMatrix("wg vector (rate)", wg);

    ColumnVector deltag(R);
    for (int r=0; r < deltag.Nrows(); ++r)
        deltag.element(r) = rng.GetRandomDouble();
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

    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
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
                macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part1);

                //Matrix temp_matrix = tobeinversed.i() * (xg.t() * w_div_delta);
                Matrix temp_matrix = tobeinversed.i() * xgsigmaw;

                macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part1);
                if (temp_matrix.element(0,0) < 0) {
                    macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_combine);
                    temp_matrix = xg * temp_matrix;
                    //printMatrix("temp_matrix matrix", temp_matrix);
                    temp_matrix = wg - temp_matrix;
                    //printMatrix("temp_matrix matrix", temp_matrix);
                    temp_matrix = SP(temp_matrix, temp_matrix);
                    matrix_best.element(i,j) = -log((deltag.t() * temp_matrix).AsScalar());
                    macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_combine);
                }
            }
        }
    }
    printMatrix("final matrix_best matrix", matrix_best);
    ReportTimeEstimate(StartTime, Reps, 1, &Console);
    macroRunTimeManagerPrint(stdout);
}

#define TNT_BOUNDS_CHECK

#include "JAMA.h"
#include "tnt.h"

void printMatrix(const std::string& s, const Array2D<double>& m) {
   /*std::cout << s  << std::endl;
    for (int r=0; r < m.dim1(); ++r) {
        std::cout << "row " << r + 1 << ":\t";
        for (int c=0; c < m.dim2(); ++c)
            std::cout << m[r][c] << "\t";
        std::cout << std::endl;
    }
    std::cout << std::endl;*/
}

void printMatrix(const std::string& s, const Array1D<double>& m) {
    /*std::cout << s  << std::endl;
    for (int r=0; r < m.dim1(); ++r) {
        std::cout << "row " << r + 1 << ":\t" << m[r] << "\t" << std::endl;
    }
    std::cout << std::endl;*/
}

void GetInverse(const Array2D<double>& A, Array2D<double>& invA)
{
   Array2D<double> I(A.dim1(), A.dim2(), 0.0);
   for (int i = 0;i < A.dim1(); i++)
      I[i][i] = 1.0;
   JAMA::QR<double> qr(A);
   invA = qr.solve(I);
}

void GetInverseLU(const Array2D<double>& A, Array2D<double>& invA)
{
   Array2D<double> I(A.dim1(), A.dim2(), 0.0);
   for (int i = 0;i < A.dim1(); i++)
      I[i][i] = 1.0;
   JAMA::LU<double> lu(A);
   invA = lu.solve(I);
}

void getTranspose(const Array2D<double>& src, Array2D<double>& T) 
{

   for (int i=0; i < T.dim1(); ++i) {
       for (int j=0; j < T.dim2(); ++j) {
          double d = src[j][i];
          T[i][j] = src[j][i];
       }
   }
}

void MatVecMult(const Array2D<double> &A, const Array1D<double> &B, Array1D<double>& C)
{
   for (int i = 0; i < C.dim(); i++)
      {
      C[i] = 0;
      for (int j = 0; j < B.dim(); j++)
         C[i] += A[i][j]*B[j];
      }
}

double MatVectorsMult(const Array1D<double> &A, const Array1D<double> &B)
{
   double product=0;
   for (int i = 0; i < A.dim(); i++)
      {
      product += A[i]*B[i];
      }
   return product;
}

void testOptimized_TntJAMA_lib_Algorithm(PrintScreen& Console, int R, int C, int Max, int Reps) {
    Console.Printf("\n\nRunning testOptimized_TntJAMA_lib_Algorithm: Rows: %d, Cols: %d, Max: %d, Proj Reps: %d\n\n", BasePrint::P_STDOUT, R, C, Max, Reps);

    RandomNumberGenerator rng;

    macroRunTimeManagerInit();

    Array2D<double> xg(R,C);
    int counter = 0;
    for (int r=0; r < xg.dim1(); ++r)
        for (int c=0; c < xg.dim2(); ++c) {
            if (c == 0)
                xg[r][c] = 0;
            else if (c == 1)
                xg[r][c] = 1;
            else
                xg[r][c] = rng.GetRandomDouble() * 1000;
        }
    printMatrix("xg matrix", xg);

    Array1D<double> wg(R);
    for (int r=0; r < wg.dim(); ++r)
        wg[r] = rng.GetRandomDouble() * 100;
    printMatrix("wg vector (rate)", wg);

    Array1D<double> deltag(R);
    for (int r=0; r < deltag.dim(); ++r)
        deltag[r] = rng.GetRandomDouble();
    printMatrix("deltag vector (weight)", deltag);

    Array1D<double> w_div_delta(R);
    for (int r=0; r < w_div_delta.dim(); ++r)
        w_div_delta[r] = wg[r]/deltag[r];
    printMatrix("w_div_delta vector (weight/delta)", w_div_delta);

    Array2D<double> matrix_best(R,R/*???*/, -std::numeric_limits<double>::max());
    printMatrix("initial matrix_best matrix", matrix_best);

    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    for (int i=0; i < R; ++i) {         
        // reset all column elements of first column to zero
        for (int r=0; r < xg.dim1(); ++r) {
            xg[r][0] = 0;
        }
        printMatrix("xg matrix (reset)",xg);
        
        Array2D<double> tobeinversed = xg.copy();
        for (int r=0; r < tobeinversed.dim1(); ++r)
            for (int c=0; c < tobeinversed.dim2(); ++c) {
                tobeinversed[r][c] = tobeinversed[r][c]/deltag[r];
            }
        printMatrix("tobeinversed matrix", tobeinversed);
        Array2D<double> xg_trans(xg.dim2(), xg.dim1());
        getTranspose(xg, xg_trans);
        printMatrix("xg.t() matrix", xg_trans);
        tobeinversed = matmult(xg_trans, tobeinversed);
        printMatrix("xg.t() * tobeinversed matrix", tobeinversed);

        Array1D<double> xgsigmaw(C);   
        MatVecMult(xg_trans, w_div_delta, xgsigmaw);
        printMatrix("xgsigmaw matrix", xgsigmaw);

        int mincluster=10;
        for (int j=0; j < Max; ++j) {
            // add neighbor case to collection
            int rand_neighbor = j;//Equilikely(0L,R-1,rng);
            xg[rand_neighbor][0] = 1;
            printMatrix("xg matrix (adding neighbor)",xg);

            tobeinversed[0][0] = tobeinversed[0][0] + (1/deltag[rand_neighbor]);
            for (int k=1; k < tobeinversed.dim1(); ++k) {
                tobeinversed[0][k] = tobeinversed[0][k] + xg[rand_neighbor][k]/deltag[rand_neighbor];
                tobeinversed[k][0] = tobeinversed[0][k];
            }
            printMatrix("tobeinversed matrix",tobeinversed);

            xgsigmaw[0] = xgsigmaw[0] + w_div_delta[rand_neighbor];

            if (j >= mincluster /* dist[i,j] < dist[i,j+1] */) {
                macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_part1);
                
                //Array1D<double> ResultVec(C), temp_matrix(C);
                //getTranspose(xg, xg_trans);
                //MatVecMult(xg_trans, w_div_delta, ResultVec); // result is 4 x 20 * 20 x 1 => 4 x 1

                Array2D<double> tobeinversed_inversed(C, C, 0.0);
                GetInverseLU(tobeinversed, tobeinversed_inversed);
                if (tobeinversed_inversed.dim1() == 0) {
                    printf("singular detected\n");
                    return;
                }

                Array1D<double> temp_matrix(C);
                MatVecMult(tobeinversed_inversed, xgsigmaw, temp_matrix);

                macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_part1);
                if (temp_matrix[0] < 0) {
                    macroRunTimeStartFocused(FocusRunTimeComponent::BetaCalculation_combine);
                    Array1D<double> tempVec(R);
                    MatVecMult(xg, temp_matrix, tempVec);
                    temp_matrix = wg - tempVec;
                    temp_matrix = temp_matrix * temp_matrix;
                    printMatrix("temp_matrix matrix", temp_matrix);
                    matrix_best[i][j] = -log(MatVectorsMult(deltag, temp_matrix));
                    macroRunTimeStopFocused(FocusRunTimeComponent::BetaCalculation_combine);
                }
            }
        }
    }
    printMatrix("final matrix_best matrix", matrix_best);
    ReportTimeEstimate(StartTime, Reps, 1, &Console);
    macroRunTimeManagerPrint(stdout);
}


int main(int argc, char *argv[]) {
  int                   ii;
  bool                  bExecuting;
  time_t                RunTime;
  CParameters           Parameters;
  std::string           sMessage;
  PrintScreen           Console(false);

  try {
    __SaTScanInit(argv[0]);
      
    Console.Printf("\n\nTesting weighted normal with covariates algorithms ...\n", BasePrint::P_STDOUT);


    int R = 35, C = 4, Max=R/2, Reps=1000;
    //testLongAlgorithm(Console, R, C, Max, Reps);
    //testOptimizedAlgorithm(Console, R, C, Max, Reps);
    //testOptimized_TntJAMA_lib_Algorithm(Console, R, C, Max, Reps);

    R = 500, Max=R/2;
    //testLongAlgorithm(Console, R, C, Max, Reps);
    //testOptimizedAlgorithm(Console, R, C, Max, Reps);
    //testOptimized_TntJAMA_lib_Algorithm(Console, R, C, Max, Reps);

    R = 1000, Max=R/2;
    //testLongAlgorithm(Console, R, C, Max, Reps);
    //testOptimizedAlgorithm(Console, R, C, Max, Reps);
    //testOptimized_TntJAMA_lib_Algorithm(Console, R, C, Max, Reps);

    
    R = 3000, Max=R/2;
    //testLongAlgorithm(Console, R, C, Max, Reps);
    //testOptimizedAlgorithm(Console, R, C, 1000, Reps);    
    //testOptimized_TntJAMA_lib_Algorithm(Console, R, C, Max, Reps);

    //return 0;


    Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
    bExecuting = !validateCommandLineArguments(argc, argv);
    time(&RunTime); //get start time
    if (!ParameterAccessCoordinator(Parameters).Read(argv[1], Console))
      throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                             "Please review above message(s) and modify parameter settings accordingly.");
    if ((ii = getCommandLineArgumentIndex(argc, argv, "-o")) != 0)
      Parameters.SetOutputFileName(argv[++ii]); // overide parameter filename, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-one-cpu"))
      Parameters.SetNumParallelProcessesToExecute(1); //override parameter file setting, if requested
    Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
    Parameters.SetRunHistoryFilename(AppToolkit::getToolkit().GetRunHistoryFileName());
    //validate parameters - print errors to console
    if (!ParametersValidate(Parameters).Validate(Console))
      throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                             "Please review above message(s) and modify parameter settings accordingly.");
    if (getCommandLineArgumentIndex(argc, argv, "-centric") && Parameters.GetPermitsCentricExecution())
      Parameters.SetExecutionType(CENTRICALLY); // overide execution type, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-all-out"))
      Parameters.RequestAllAdditionalOutputFiles(); // overide selected output files, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-p"))
      ParametersPrint(Parameters).Print(stdout);
    if (getCommandLineArgumentIndex(argc, argv, "-c"))
      Console.Printf("Parameters confirmed.\n", BasePrint::P_STDOUT);
    if (bExecuting) {
      //create analysis runner object and execute analysis
      AnalysisRunner(Parameters, RunTime, Console);
      //report completion
      Console.Printf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",
                     BasePrint::P_STDOUT, Parameters.GetOutputFileName().c_str());
    }
    __SaTScanExit();
  }
  catch (resolvable_error & x) {
    Console.Printf("%s\n\nJob cancelled.", BasePrint::P_ERROR, x.what());
    __SaTScanExit();
    return 1;
  }
  catch (usage_error & x) {
    Console.Printf(x.what(), BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  catch (prg_exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), x.trace());
    __SaTScanExit();
    return 1;
  }
  catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  catch (std::exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), "Callpath not available.");
    __SaTScanExit();
    return 1;
  }
  catch (BaseException& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), "Callpath not available.");
    __SaTScanExit();
    return 1;
  }
  catch (...) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "Unknown program error encountered.", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  return 0;
} /* main() */

