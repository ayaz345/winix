//
// Created by bruce on 9/05/20.
//
#include "../fs/fs.h"
#include "unit_test.h"
#include <assert.h>

const char * dirent_array[] = {
        ".",
        "..",
        "bar.txt",
        "bar2.txt"
};

const char *FILE1 = "/foo.txt";
const char *FILE2 = "/foo2.txt";
const char *DIR_NAME = "/dir/";
const char *DIR_FILE1 = "/dir/bar.txt";
const char *DIR_FILE2 = "/dir/bar2.txt";
const char *TTY_PATH = "/tty";
const int TTY_DEV = MAKEDEV(3, 1);
char buffer[PAGE_LEN];
char buffer2[PAGE_LEN];

int file_size(struct proc* who, int fd){
    struct stat statbuf;
    (void)sys_fstat(who, fd, &statbuf);
    return statbuf.st_size;
}

void _reset_fs(){
    init_disk();
    init_dev();
    init_fs();
    init_tty();
    init_drivers();
    mock_init_proc();
}

void _close_delete_file(int fd, char *name){
    int ret;
    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);
}

void test_given_open_when_flag_is_o_create_should_return_0(){
    int fd;
    
    fd = sys_open(curr_scheduling_proc, FILE1 ,O_CREAT, O_RDWR);
    assert(fd == 0);

    _close_delete_file(fd, FILE1);
}

void test_given_creat_when_file_not_present_should_return_0(){
    int fd;
    
    fd = sys_creat(curr_scheduling_proc, FILE1 , O_RDWR);
    assert(fd == 0);

    _close_delete_file(fd, FILE1);
}

void test_given_close_when_file_closed_should_return_ebadf(){
    int fd, ret;
    
    fd = sys_open(curr_scheduling_proc, FILE1 ,O_CREAT, O_RDWR);
    assert(fd == 0);

    _close_delete_file(fd, FILE1);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == EBADF);
}

void test_given_pipe_read_when_fd_is_closed_return_ebadf(){
    int ret, fd;
    
    fd = sys_creat(curr_scheduling_proc, FILE1 , O_RDWR);
    assert(fd == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_read(curr_scheduling_proc, fd, buffer, PAGE_LEN);
    assert(ret == EBADF);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);
}

void test_given_pipe_write_when_fd_is_closed_return_ebadf(){
    int ret, fd;
    
    fd = sys_creat(curr_scheduling_proc, FILE1 , O_RDWR);
    assert(fd == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_write(curr_scheduling_proc, fd, buffer, PAGE_LEN);
    assert(ret == EBADF);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);
}

void test_given_opening_file_when_deleting_file_should_return_error(){
    int ret, fd;
    
    fd = sys_creat(curr_scheduling_proc, FILE1 , O_RDWR);
    assert(fd == 0);

    _close_delete_file(fd, FILE1);

    ret = sys_open(curr_scheduling_proc, FILE1 , O_RDWR, 0775);
    assert(ret == ENOENT);
}

void test_given_two_file_descriptors_when_dupping_file_should_behave_the_same(){
    int ret, fd, fd2;
    
    fd = sys_open(curr_scheduling_proc, FILE1 ,O_CREAT | O_RDWR, 0775);
    assert(fd == 0);
    
    fd2 = sys_dup(curr_scheduling_proc, fd);
    assert(fd2 == fd + 1);

    ret = sys_write(curr_scheduling_proc, fd, "abc", 3);
    assert(ret == 3);

    ret = sys_write(curr_scheduling_proc, fd, "def", 4);
    assert(ret == 4);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 100);
    assert(ret == 0);

    ret = sys_read(curr_scheduling_proc, fd2, buffer, 100);
    assert(ret == 0);

    ret = sys_lseek(curr_scheduling_proc, fd2, 0, SEEK_SET);
    assert(ret == 0);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 100);
    assert(ret == 7);
    assert(strcmp(buffer, "abcdef") == 0);

    ret = sys_lseek(curr_scheduling_proc, fd, 0, SEEK_SET);
    assert(ret == 0);

    ret = sys_read(curr_scheduling_proc, fd2, buffer, 100);
    assert(ret == 7);
    assert(strcmp(buffer, "abcdef") == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_close(curr_scheduling_proc, fd2);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);
}

void test_given_read_when_open_and_closing_file_should_persistted_data(){
    int ret, fd;

    fd = sys_open(curr_scheduling_proc, FILE1 , O_CREAT | O_RDONLY, 0x0775);
    assert(fd == 0);
    
    ret = sys_write(curr_scheduling_proc, fd, "abcdef", 7);
    assert(ret == 7);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    fd = sys_open(curr_scheduling_proc, FILE1 , O_RDONLY, 0x0775);
    assert(file_size(curr_scheduling_proc, fd) == 7);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 100);
    assert(ret == 7);
    assert(strcmp(buffer, "abcdef") == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);
}

void _init_pipe(int pipe_fd[2], struct proc* pcurr2){
    int ret;
    pcurr2->pid = 2;
    pcurr2->proc_nr = 2;

    ret = sys_pipe(curr_scheduling_proc, pipe_fd);
    assert(ret == 0);

    emulate_fork(curr_scheduling_proc, pcurr2);
}

void __close_pipe(int pipe_fd[2], struct proc* process, int num){
    sys_close(process, pipe_fd[num]);
    sys_close(process, pipe_fd[num]);
}

void _close_pipe(int pipe_fd[2], struct proc* pcurr2){
    __close_pipe(pipe_fd, curr_scheduling_proc, 0);
    __close_pipe(pipe_fd, pcurr2, 1);
}

void test_given_pipe_read_when_no_data_in_pipe_should_return_suspend(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer, 100);
    assert(ret == SUSPEND);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_write_when_no_data_in_pipe_should_return_succeed(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_write(curr_scheduling_proc, pipe_fd[1], "a", 1);
    assert(ret == 1);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_write_when_pipe_is_full_should_return_suspend(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_write(curr_scheduling_proc, pipe_fd[1], buffer, PAGE_LEN);
    assert(ret == PAGE_LEN);

    ret = sys_write(curr_scheduling_proc, pipe_fd[1], buffer, PAGE_LEN);
    assert(ret == SUSPEND);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_read_when_proc_was_suspended_should_return(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);
    memset(buffer, 0, PAGE_LEN);

    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer, 100);
    assert(ret == SUSPEND);
    assert(*buffer == 0);

    ret = sys_write(&pcurr2, pipe_fd[1], "1234", 5);
    assert(ret == 5);
    assert(strcmp(buffer, "1234") == 0);

    ret = sys_write(&pcurr2, pipe_fd[1], "5678", 5);
    assert(ret == 5);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_read_when_data_is_written_should_return_data(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_write(&pcurr2, pipe_fd[1], "5678", 5);
    assert(ret == 5);

    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer, 100);
    assert(ret == 5);
    assert(strcmp(buffer, "5678") == 0);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_read_when_pipe_is_full_should_return_data(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    memset(buffer, 'a', PAGE_LEN - 1);
    buffer[PAGE_LEN - 1] = 0;

    ret = sys_write(&pcurr2, pipe_fd[1], buffer, PAGE_LEN);
    assert(ret == PAGE_LEN);

    ret = sys_write(&pcurr2, pipe_fd[1], "abc", 4);
    assert(ret == SUSPEND);

    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer2, PAGE_LEN);
    assert(ret == PAGE_LEN);
    assert(strcmp(buffer, buffer2) == 0);

    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer2, PAGE_LEN);
    assert(ret == 4);
    assert(strcmp(buffer2, "abc") == 0);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_write_when_one_read_fd_s_closed_should_return_success(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);
    memset(buffer, 0, PAGE_LEN);

    ret = sys_close(curr_scheduling_proc, pipe_fd[0]);
    assert(ret == 0);

    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == 2);

    ret = sys_read(&pcurr2, pipe_fd[0], buffer, 2);
    assert(ret == 2);
    assert(strcmp("a", buffer) == 0);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_write_when_read_fd_are_closed_should_return_sigpipe(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_close(curr_scheduling_proc, pipe_fd[0]);
    assert(ret == 0);

    ret = sys_close(&pcurr2, pipe_fd[0]);
    assert(ret == 0);
    
    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == SUSPEND);
    assert(pcurr2.sig_pending & (1 << SIGPIPE));

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_pipe_write_when_read_fd_closed_and_sigpipe_ignored_should_return_epipe(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    _init_pipe(pipe_fd, &pcurr2);

    ret = sys_close(curr_scheduling_proc, pipe_fd[0]);
    assert(ret == 0);

    ret = sys_close(&pcurr2, pipe_fd[0]);
    assert(ret == 0);
    
    pcurr2.sig_table[SIGPIPE].sa_handler = SIG_IGN;
    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == EPIPE);

    _close_pipe(pipe_fd, &pcurr2);
}

void test_given_access_when_file_not_exist_should_return_enoent(){
    int ret = sys_access(curr_scheduling_proc, FILE1, F_OK);
    assert(ret == ENOENT);
}

void test_given_access_when_file_exists_should_return_0(){
    int ret;
    ret = sys_creat(curr_scheduling_proc, FILE1, 0x755);
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, FILE1, F_OK);
    assert(ret == 0);

    _close_delete_file(0, FILE1);
}

void test_given_access_when_folder_exists_should_return_0(){
    int ret;
    ret = sys_mkdir(curr_scheduling_proc, DIR_NAME, 0x755);
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, DIR_NAME, F_OK);
    assert(ret == 0);

    _reset_fs();
}

void test_given_access_when_under_folder_should_return_enoent(){
    int ret;
    ret = sys_mkdir(curr_scheduling_proc, DIR_NAME, 0x755);
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, DIR_FILE1, F_OK);
    assert(ret == ENOENT);

    _reset_fs();
}

void test_given_link_stat_when_two_files_are_linked_should_return_same(){
    int fd, ret;
    struct stat statbuf, statbuf2;

    fd = sys_creat(curr_scheduling_proc, FILE1, O_RDWR);
    assert(fd == 0);

    ret = sys_link(curr_scheduling_proc, FILE1, FILE2);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, FILE1, &statbuf);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, FILE2, &statbuf2);
    assert(ret == 0);
    assert(statbuf.st_ino == statbuf2.st_ino);
    assert(statbuf.st_dev == statbuf2.st_dev);
    assert(statbuf.st_nlink == 2);

    _reset_fs();
}


void test_given_link_stat_when_one_file_deleted_should_return_1_nlink(){
    int fd, ret;
    struct stat statbuf;

    fd = sys_creat(curr_scheduling_proc, FILE1, O_RDWR);
    assert(fd == 0);

    ret = sys_link(curr_scheduling_proc, FILE1, FILE2);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, FILE1, &statbuf);
    assert(ret == 0);
    assert(statbuf.st_nlink == 2);

    ret = sys_unlink(curr_scheduling_proc, FILE1);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, FILE2, &statbuf);
    assert(ret == 0);
    assert(statbuf.st_nlink == 1);

    _reset_fs();
}

void test_given_chdir_when_dir_not_present_should_return_eexist(){
    int ret = sys_chdir(curr_scheduling_proc, "/not_exist");
    assert(ret == EEXIST);
}

void test_given_chdir_when_path_is_file_should_return_eexist(){
    int fd = sys_creat(curr_scheduling_proc, FILE1, O_RDWR);
    assert(fd == 0);

    int ret = sys_chdir(curr_scheduling_proc, FILE1);
    assert(ret == ENOTDIR);

    _close_delete_file(fd, FILE1);
}

void test_given_chdir_when_dir_is_valid_should_succeed(){
    struct stat statbuf;
    int ret = sys_mkdir(curr_scheduling_proc, DIR_NAME, O_RDWR);
    assert(ret == 0);

    ret = sys_chdir(curr_scheduling_proc, DIR_NAME);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, DIR_NAME, &statbuf);
    assert(ret == 0);
    assert(curr_scheduling_proc->fp_workdir->i_num == statbuf.st_ino);

    _reset_fs();
}

void test_given_chmod_when_file_not_present_should_return_enonent(){
    int ret = sys_chmod(curr_scheduling_proc, "/notexists", O_RDONLY);
    assert(ret == ENOENT);
}

void test_given_chmod_stat_when_file_present_should_return_0(){
    int fd, ret;
    struct stat statbuf;

    fd = sys_creat(curr_scheduling_proc, FILE1, O_RDWR);
    assert(fd == 0);

    ret = sys_chmod(curr_scheduling_proc, FILE1, 0x777);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, FILE1, &statbuf);
    assert(ret == 0);
    assert(statbuf.st_mode == 0x777);

    _close_delete_file(fd, FILE1);
}

void test_given_chmod_stat_when_folder_present_should_return_0(){
    int fd, ret;
    struct stat statbuf;

    fd = sys_mkdir(curr_scheduling_proc, DIR_NAME, O_RDWR);
    assert(fd == 0);

    ret = sys_chmod(curr_scheduling_proc, DIR_NAME, 0x777);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, DIR_NAME, &statbuf);
    assert(ret == 0);
    assert(statbuf.st_mode == 0x777);

    _reset_fs();
}

void test_given_getdents_when_no_files_in_folder_should_return_default_files(){
    struct dirent dir[5];
    int i;
    int fd = sys_open(curr_scheduling_proc, "/", 0, O_RDONLY);
    int ret = sys_getdents(curr_scheduling_proc, fd, dir, 5);
    assert(ret == sizeof(struct dirent) * 2);
    for(i = 0; i < 2; i++){
        assert(char32_strcmp(dir[i].d_name, dirent_array[i]) == 0);
    }

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);
}

void test_given_getdents_when_files_in_folder_should_return_files(){
    int ret, fd, fd2, i;
    struct dirent dir[5];

    ret = sys_mkdir(curr_scheduling_proc, DIR_NAME, O_RDWR);
    assert(ret == 0);

    fd = sys_creat(curr_scheduling_proc, DIR_FILE1, O_RDWR);
    assert(fd == 0);

    ret = sys_link(curr_scheduling_proc, DIR_FILE1, DIR_FILE2);
    assert(ret == 0);

    fd2 = sys_open(curr_scheduling_proc, DIR_NAME, O_RDONLY, 0);
    assert(fd2 == 1);

    ret = sys_getdents(curr_scheduling_proc, fd2, dir, 5);
    size_t dirent_size = sizeof(struct dirent);
    size_t desired = dirent_size * 4;
    assert(ret == desired);
    for (i = 0; i < 4; ++i) {
        assert(char32_strcmp(dir[i].d_name, dirent_array[i]) == 0);
    }

    ret = sys_getdents(curr_scheduling_proc, fd2, dir, 10);
    assert(ret == 0);

    _reset_fs();
}

void test_given_mknod_when_path_valid_should_return_0(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    _reset_fs();
}

void test_given_dev_open_when_file_is_driver_should_return_from_driver(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    TTY_OPEN_CALLED = false;

    int fd = sys_open(curr_scheduling_proc, TTY_PATH, O_EXCL, O_RDWR);
    assert(fd == 0);
    assert(TTY_OPEN_CALLED == true);

    _reset_fs();
}

void test_given_dev_read_when_file_is_driver_should_return_from_driver(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    int fd = sys_open(curr_scheduling_proc, TTY_PATH, O_EXCL, O_RDWR);
    assert(fd == 0);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 3);
    assert(ret == TTY_RETURN);

    _reset_fs();
}

void test_given_dev_write_when_file_is_driver_should_return_from_driver(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    int fd = sys_open(curr_scheduling_proc, TTY_PATH, O_EXCL, O_RDWR);
    assert(fd == 0);

    ret = sys_write(curr_scheduling_proc, fd, buffer, 3);
    assert(ret == TTY_RETURN);

    _reset_fs();
}

void test_given_dev_close_when_file_is_driver_should_return_from_driver(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    int fd = sys_open(curr_scheduling_proc, TTY_PATH, O_EXCL, O_RDWR);
    assert(fd == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == TTY_RETURN);

    _reset_fs();
}

void test_given_dev_dup_when_file_is_driver_should_return_from_driver(){
    int ret = sys_mknod(curr_scheduling_proc, TTY_PATH, O_RDWR, TTY_DEV);
    assert(ret == 0);

    int fd = sys_open(curr_scheduling_proc, TTY_PATH, O_EXCL, O_RDWR);
    assert(fd == 0);

    int fd2 = sys_dup2(curr_scheduling_proc, fd, 1);
    assert(1 == fd2);

    ret = sys_write(curr_scheduling_proc, fd, buffer, 3);
    assert(ret == TTY_RETURN);

    ret = sys_write(curr_scheduling_proc, fd2, buffer, 3);
    assert(ret == TTY_RETURN);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 3);
    assert(ret == TTY_RETURN);

    ret = sys_read(curr_scheduling_proc, fd2, buffer, 3);
    assert(ret == TTY_RETURN);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == TTY_RETURN);

    ret = sys_close(curr_scheduling_proc, fd2);
    assert(ret == TTY_RETURN);

    _reset_fs();
}


int main(){

    init_bitmap();
    _reset_fs();

    test_given_open_when_flag_is_o_create_should_return_0();
    test_given_creat_when_file_not_present_should_return_0();
    test_given_close_when_file_closed_should_return_ebadf();
    test_given_opening_file_when_deleting_file_should_return_error();
    test_given_two_file_descriptors_when_dupping_file_should_behave_the_same();
    test_given_read_when_open_and_closing_file_should_persistted_data();
    test_given_pipe_read_when_no_data_in_pipe_should_return_suspend();
    test_given_pipe_write_when_no_data_in_pipe_should_return_succeed();
    test_given_pipe_write_when_pipe_is_full_should_return_suspend();
    test_given_pipe_read_when_fd_is_closed_return_ebadf();
    test_given_pipe_write_when_fd_is_closed_return_ebadf();
    test_given_pipe_read_when_proc_was_suspended_should_return();
    test_given_pipe_read_when_data_is_written_should_return_data();
    test_given_pipe_read_when_pipe_is_full_should_return_data();
    test_given_pipe_write_when_read_fd_are_closed_should_return_sigpipe();
    test_given_pipe_write_when_one_read_fd_s_closed_should_return_success();
    test_given_pipe_write_when_read_fd_closed_and_sigpipe_ignored_should_return_epipe();
    test_given_access_when_file_not_exist_should_return_enoent();
    test_given_access_when_file_exists_should_return_0();
    test_given_access_when_folder_exists_should_return_0();
    test_given_access_when_under_folder_should_return_enoent();
    test_given_link_stat_when_two_files_are_linked_should_return_same();
    test_given_link_stat_when_one_file_deleted_should_return_1_nlink();
    test_given_chdir_when_dir_not_present_should_return_eexist();
    test_given_chdir_when_path_is_file_should_return_eexist();
    test_given_chdir_when_dir_is_valid_should_succeed();
    test_given_chmod_when_file_not_present_should_return_enonent();
    test_given_chmod_stat_when_file_present_should_return_0();
    test_given_chmod_stat_when_folder_present_should_return_0();
    test_given_getdents_when_no_files_in_folder_should_return_default_files();
    test_given_getdents_when_files_in_folder_should_return_files();
    test_given_mknod_when_path_valid_should_return_0();
    test_given_dev_open_when_file_is_driver_should_return_from_driver();
    test_given_dev_read_when_file_is_driver_should_return_from_driver();
    test_given_dev_write_when_file_is_driver_should_return_from_driver();
    test_given_dev_close_when_file_is_driver_should_return_from_driver();
    test_given_dev_dup_when_file_is_driver_should_return_from_driver();

    printf("filesystem unit test passed\n");
    return 0;
}

