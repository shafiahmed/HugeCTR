/*
 * Copyright (c) 2020, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "HugeCTR/include/session.hpp"
#include <cuda_profiler_api.h>
#include "HugeCTR/include/data_parser.hpp"
#include "gtest/gtest.h"
#include "utest/test_utils.h"

using namespace HugeCTR;

TEST(session_test, basic_session) {
  const int batchsize = 2048;
  const int label_dim = 1;
  test::mpi_init();
  {
    // generate data
    // note: the parameters should match <configure>.json file
    const long long dense_dim = 64;
    const int max_nnz = 30;
    typedef long long T;
    const int vocabulary_size = 1603616;
    const std::string prefix("./simple_sparse_embedding/simple_sparse_embedding");
    const std::string file_list_name = prefix + "_file_list.txt";
    const int num_files = 3;
    const long long num_records = batchsize * 5;
    const long long slot_num = 20;
    HugeCTR::data_generation<T, Check_t::Sum>(file_list_name, prefix, num_files, num_records,
                                              slot_num, vocabulary_size, label_dim, dense_dim,
                                              max_nnz);
  }

  std::string json_name = PROJECT_HOME_ + "utest/simple_sparse_embedding.json";
  Session session_instance(json_name);
  cudaProfilerStart();
  for (int i = 0; i < 100; i++) {
    session_instance.train();
    if (i % 10 == 0) {
      float loss = 0;
      session_instance.get_current_loss(&loss);
      std::cout << "iter:" << i << "; loss: " << loss << std::endl;
    }
  }
  cudaProfilerStop();
}
