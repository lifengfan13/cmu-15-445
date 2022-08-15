//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

#include "common/logger.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {}

void InsertExecutor::Init() {
    auto catalog = exec_ctx_->GetCatalog();
    table_info_ = catalog->GetTable(plan_->TableOid());
    indexes_ = catalog->GetTableIndexes(table_info_->name_);
}

auto InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool { 
    if(plan_->IsRawInsert()) {
        auto raw_vals = plan_->RawValues();
        for(auto &values: raw_vals) {
            Tuple tmp_tuple(values,&table_info_->schema_);
            // LOG_DEBUG("Insert a Tuple %s", tmp_tuple.ToString(&table_info_->schema_).c_str());
            if(!table_info_->table_->InsertTuple(tmp_tuple,rid,exec_ctx_->GetTransaction()))
                throw Exception("Insert a Tuple faild:" + tmp_tuple.ToString(&table_info_->schema_));

            // update index 
            for(auto index:indexes_) {
                auto meta = index->index_->GetMetadata();
                auto key_tuple = tmp_tuple.KeyFromTuple(table_info_->schema_, index->key_schema_, meta->GetKeyAttrs());
                index->index_->InsertEntry(key_tuple, *rid, exec_ctx_->GetTransaction()); 
            }  
        }
    }else{
        while (child_executor_->Next(tuple, rid)) { 
            // LOG_DEBUG("Insert a Tuple %s", tuple->ToString(&table_info_->schema_).c_str());
            if(!table_info_->table_->InsertTuple(*tuple,rid,exec_ctx_->GetTransaction()))
                throw Exception("Insert a Tuple faild:" + tuple->ToString(&table_info_->schema_));
            // update index 
            for(auto index:indexes_) {
                auto meta = index->index_->GetMetadata();
                auto key_tuple = tuple->KeyFromTuple(table_info_->schema_, index->key_schema_, meta->GetKeyAttrs());
                index->index_->InsertEntry(key_tuple, *rid, exec_ctx_->GetTransaction()); 
            }  
        }
    }
    return false; 
}

}  // namespace bustub
