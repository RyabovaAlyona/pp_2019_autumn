// Copyright 2019 Ryabova Alyona

#include <mpi.h>
#include <algorithm>
#include <iostream>
#include "../../../modules/task_3/ryabova_a_shell_sort/shell_sort.h"

std::vector<int> compare(std::vector<int> array) {

    int sizeArray = array.size();
    int k;
    for (int i = 1; i < sizeArray; i++) {
        k = 1;
        while ((i - k >= 0) && (array[i - k + 1] < array[i - k])) {
            int n = array[i - k + 1];
            array[i - k + 1] = array[i - k];
            array[i - k] = n;
            k++;
        }
    }
    return array;
}

std::vector<int> shell_sort(std::vector<int> array) {
    int ProcNum, ProcRank;
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (array.empty())
        return array;

    int sizeArray = array.size();

    if (sizeArray < 2)
        return array;
    if (ProcNum == 1)
        return compare(array);

    int op = array.size();
    int sizeLocalArray, countOp, countProc, count, count2;
    int n = 0, countProcNum = 0;

    while (op != 0) {

        op = op / 2;
        countOp = op / (ProcNum - 1);
        countProc = op % (ProcNum - 1);
        count = op < ProcNum ? op : ProcNum;
        count2 = countOp;
        if (countProc > 0)
            count2++;
        n = 0, countProcNum = 0;
        sizeLocalArray = sizeArray / op;
        std::vector<int> localArray(sizeLocalArray);

        do {
            if (countProcNum + countProc == op)
                count = countProc + 1;
            for (int proc = 0; proc < count - 1; proc++) {
                if (ProcRank == 0) {
                    localArray.clear();
                    for (int i = 0; i < sizeLocalArray; i++) {
                        localArray.push_back(array[proc + countProcNum + op * i]);
                    }
                    MPI_Send(&localArray[0], sizeLocalArray, MPI_INT, proc + 1, n, MPI_COMM_WORLD);
                }
                else {
                    if (ProcRank == proc + 1) {
                        MPI_Status status;
                        MPI_Recv(&localArray[0], sizeLocalArray, MPI_INT, 0,
                            n, MPI_COMM_WORLD, &status);
                        localArray = compare(localArray);
                        MPI_Send(&localArray[0], sizeLocalArray, MPI_INT, 0, proc + 1 + n, MPI_COMM_WORLD);
                    }
                }
            }
            countProcNum += ProcNum - 1;
            n++;
        } while (countProcNum < op);

        count = op < ProcNum ? op : ProcNum;
        countProcNum = 0;
        n = 0;

        if (ProcRank == 0) {
            do {
                if (countProcNum + countProc == op)
                    count = countProc + 1;
                for (int proc = 0; proc < count - 1; proc++) {
                    MPI_Status status;
                    MPI_Recv(&localArray[0], sizeLocalArray, MPI_INT, proc + 1,
                        proc + 1 + n, MPI_COMM_WORLD, &status);
                    for (int i = 0; i < sizeLocalArray; i++) {
                        array[proc + op * i + countProcNum] = localArray[i];
                    }
                }
                countProcNum = countProcNum + ProcNum - 1;
                n++;
            } while (countProcNum < op);
        }
        if (sizeLocalArray == sizeArray)
            op = 0;
    }
    return array;
}
