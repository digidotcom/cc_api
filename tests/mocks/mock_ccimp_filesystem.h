#ifndef _MOCK_CCIMP_FILESYSTEM_H_
#define _MOCK_CCIMP_FILESYSTEM_H_

void Mock_ccimp_fs_file_open_create(void);
void Mock_ccimp_fs_file_open_destroy(void);
void Mock_ccimp_fs_file_open_expectAndReturn(ccimp_fs_file_open_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_file_read_create(void);
void Mock_ccimp_fs_file_read_destroy(void);
void Mock_ccimp_fs_file_read_expectAndReturn(ccimp_fs_file_read_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_file_write_create(void);
void Mock_ccimp_fs_file_write_destroy(void);
void Mock_ccimp_fs_file_write_expectAndReturn(ccimp_fs_file_write_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_file_seek_create(void);
void Mock_ccimp_fs_file_seek_destroy(void);
void Mock_ccimp_fs_file_seek_expectAndReturn(ccimp_fs_file_seek_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_file_close_create(void);
void Mock_ccimp_fs_file_close_destroy(void);
void Mock_ccimp_fs_file_close_expectAndReturn(ccimp_fs_file_close_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_file_remove_create(void);
void Mock_ccimp_fs_file_remove_destroy(void);
void Mock_ccimp_fs_file_remove_expectAndReturn(ccimp_fs_file_remove_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_dir_open_create(void);
void Mock_ccimp_fs_dir_open_destroy(void);
void Mock_ccimp_fs_dir_open_expectAndReturn(ccimp_fs_dir_open_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_dir_read_entry_create(void);
void Mock_ccimp_fs_dir_read_entry_destroy(void);
void Mock_ccimp_fs_dir_read_entry_expectAndReturn(ccimp_fs_dir_read_entry_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_dir_entry_status_create(void);
void Mock_ccimp_fs_dir_entry_status_destroy(void);
void Mock_ccimp_fs_dir_entry_status_expectAndReturn(ccimp_fs_dir_entry_status_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_dir_close_create(void);
void Mock_ccimp_fs_dir_close_destroy(void);
void Mock_ccimp_fs_dir_close_expectAndReturn(ccimp_fs_dir_close_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_hash_status_create(void);
void Mock_ccimp_fs_hash_status_destroy(void);
void Mock_ccimp_fs_hash_status_expectAndReturn(ccimp_fs_hash_status_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_hash_file_create(void);
void Mock_ccimp_fs_hash_file_destroy(void);
void Mock_ccimp_fs_hash_file_expectAndReturn(ccimp_fs_hash_file_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_error_desc_create(void);
void Mock_ccimp_fs_error_desc_destroy(void);
void Mock_ccimp_fs_error_desc_expectAndReturn(ccimp_fs_error_desc_t * expect, ccimp_status_t retval);

void Mock_ccimp_fs_session_error_create(void);
void Mock_ccimp_fs_session_error_destroy(void);
void Mock_ccimp_fs_session_error_expectAndReturn(ccimp_fs_session_error_t * expect, ccimp_status_t retval);

#endif
