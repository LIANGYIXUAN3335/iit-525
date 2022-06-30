CC = gcc
CFLAGS  = -g -Wall 
default: assign_3_test1

assign_3_test1 :test_assign3_1.c dberror.c dberror.h expr.c expr.h record_mgr.c record_mgr.h rm_serializer.c storage_mgr.c storage_mgr.h buffer_mgr.c buffer_mgr.h buffer_mgr_stat.c buffer_mgr_stat.h
	gcc -o assign_3_test1 test_assign3_1.c dberror.c expr.c record_mgr.c rm_serializer.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c
assign_3_test2 : test_expr.c dberror.c dberror.h expr.c expr.h record_mgr.c record_mgr.h rm_serializer.c storage_mgr.c storage_mgr.h buffer_mgr.c buffer_mgr.h buffer_mgr_stat.c buffer_mgr_stat.h
	gcc -o assign_3_test2 test_expr.c dberror.c expr.c record_mgr.c rm_serializer.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c
clean: 
	rm assign_3_test1  assign_3_test2
run:
	./assign_3_test1
run_expr:
	./assign_3_test2