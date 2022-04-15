//
// Created by bruce on 9/05/20.
//
#include "fs.h"
#include <assert.h>

const char * dirent_array[] = {
        ".",
        "..",
        "bar.txt",
        "bar2.txt"
};

char *filename = "/foo.txt";
char buffer[BLOCK_SIZE];
char buffer2[BLOCK_SIZE];

int file_size(struct proc* who, int fd){
    struct stat statbuf;
    (void)sys_fstat(who, fd, &statbuf);
    return statbuf.st_size;
}

void test_given_o_creat_when_open_file_should_return_0(){
    int ret, fd;
    
    fd = sys_open(curr_scheduling_proc, filename ,O_CREAT | O_RDWR, 0775);
    assert(fd == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, filename);
    assert(ret == 0);
}

void test_when_creating_file_should_return_0(){
    int ret, fd;
    
    fd = sys_creat(curr_scheduling_proc, filename , O_RDWR);
    assert(fd == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, filename);
    assert(ret == 0);
}

void test_given_two_file_descriptors_when_dupping_file_should_behave_the_same(){
    int ret, fd, fd2;
    
    fd = sys_open(curr_scheduling_proc, filename ,O_CREAT | O_RDWR, 0775);
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
}

int unit_test1(){
    int ret, fd;

    fd = sys_open(curr_scheduling_proc, filename ,O_RDONLY, 0x0775);
    assert(fd == 0);
    assert(file_size(curr_scheduling_proc, fd) == 7);
    ret = sys_read(curr_scheduling_proc, fd, buffer, 100);
    assert(ret == 7);
    assert(strcmp(buffer, "abcdef") == 0);
    ret = sys_read(curr_scheduling_proc, fd, buffer, 100);
    assert(ret == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, filename);
    assert(ret == 0);

    ret = sys_open(curr_scheduling_proc, filename ,O_RDONLY, 0x0775);
    assert(ret == ENOENT); 

    return 0;
}

int unit_test2(){
    struct proc pcurr2;
    int ret;
    int pipe_fd[2];

    pcurr2.pid = 2;
    pcurr2.proc_nr = 2;

    ret = sys_pipe(curr_scheduling_proc, pipe_fd);
    assert(ret == 0);
    emulate_fork(curr_scheduling_proc, &pcurr2);
    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer, 100);
    assert(ret == SUSPEND);
    ret = sys_write(&pcurr2, pipe_fd[1], "1234", 5);
    assert(ret == 5);
    assert(strcmp(buffer, "1234") == 0);
    buffer[0] = 0;

    ret = sys_write(&pcurr2, pipe_fd[1], "1234", 5);
    assert(ret == 5);
    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer, 100);
    assert(ret == 5);
    assert(strcmp(buffer, "1234") == 0);

    memset(buffer, 'a', BLOCK_SIZE - 1);
    buffer[BLOCK_SIZE - 1] = 0;
    ret = sys_write(&pcurr2, pipe_fd[1], buffer, BLOCK_SIZE);
    assert(ret == BLOCK_SIZE);
    ret = sys_write(&pcurr2, pipe_fd[1], "abc", 4);
    assert(ret == SUSPEND);
    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer2, BLOCK_SIZE);
    assert(ret == BLOCK_SIZE);
    assert(strcmp(buffer, buffer2) == 0);
    ret = sys_read(curr_scheduling_proc, pipe_fd[0], buffer2, BLOCK_SIZE);
    assert(ret == 4);
    assert(strcmp(buffer2, "abc") == 0);

    sys_close(curr_scheduling_proc, pipe_fd[0]);
    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == 2);

    sys_close(&pcurr2, pipe_fd[0]);
    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == SUSPEND);

    pcurr2.sig_table[SIGPIPE].sa_handler = SIG_IGN;
    ret = sys_write(&pcurr2, pipe_fd[1], "a", 2);
    assert(ret == EPIPE);

    ret = sys_close(curr_scheduling_proc, pipe_fd[1]);
    assert(ret == 0);
    ret = sys_close(&pcurr2, pipe_fd[1]);
    assert(ret == 0);

    return 0;
}

int unit_test3(){
    int ret, fd, fd2, i;
    struct stat statbuf, statbuf2;
    struct dirent dir[4];

    ret = sys_access(curr_scheduling_proc, "/dev", F_OK);
    assert(ret != 0);

    ret = sys_mkdir(curr_scheduling_proc, "/dev", 0x755);
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, "/dev", F_OK);
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, "/dev/bar.txt", F_OK);
    assert(ret != 0);

    fd = sys_creat(curr_scheduling_proc, "/dev/bar.txt", 0x777);
    assert(fd == 0);

    ret = sys_access(curr_scheduling_proc, "/dev/bar.txt", F_OK);
    assert(ret == 0);

    ret = sys_chdir(curr_scheduling_proc, "/dev");
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, "/dev", &statbuf);
    assert(ret == 0);
    assert(curr_scheduling_proc->fp_workdir->i_num == statbuf.st_ino);

    ret = sys_link(curr_scheduling_proc, "/dev/bar.txt", "/dev/bar2.txt");
    assert(ret == 0);

    ret = sys_chmod(curr_scheduling_proc, "/dev/bar.txt", 0x777);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, "/dev/bar.txt", &statbuf);
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, "/dev/bar2.txt", &statbuf2);
    assert(ret == 0);
    assert(statbuf.st_ino == statbuf2.st_ino);
    assert(statbuf.st_dev == statbuf2.st_dev);
    assert(statbuf.st_nlink == 2);
    assert(statbuf.st_mode == 0x777);

    fd2 = sys_open(curr_scheduling_proc, "/dev", O_RDONLY, 0);
    assert(fd2 == 1);

    ret = sys_getdents(curr_scheduling_proc, fd2, dir, 5);
    assert(ret == sizeof(struct dirent) * 4);
    for (i = 0; i < 4; ++i) {
        assert(char32_strcmp(dir[i].d_name, dirent_array[i]) == 0);
    }
    ret = sys_getdents(curr_scheduling_proc, fd2, dir, 10);
    assert(ret == 0);

    ret = sys_unlink(curr_scheduling_proc, "/dev/bar2.txt");
    assert(ret == 0);

    ret = sys_stat(curr_scheduling_proc, "/dev/bar.txt", &statbuf);
    assert(ret == 0);
    assert(statbuf.st_nlink == 1);

    ret = sys_stat(curr_scheduling_proc, "/dev/bar2.txt", &statbuf2);
    assert(ret == ENOENT);

    ret = sys_unlink(curr_scheduling_proc, "/dev/bar.txt");
    assert(ret == 0);

    ret = sys_access(curr_scheduling_proc, "/dev/bar.txt", F_OK);
    assert(ret == ENOENT);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_close(curr_scheduling_proc, fd2);
    assert(ret == 0);
    
    return 0;
}

int unit_test_driver(){
    int ret, fd, fd2, fd3;

    ret = sys_mknod(curr_scheduling_proc, "/dev/tty", O_RDWR, MAKEDEV(3, 1));
    assert(ret == 0);

    fd = sys_open(curr_scheduling_proc, "/dev/tty", O_RDWR, 0);
    assert(fd == 0);

    ret = sys_read(curr_scheduling_proc, fd, buffer, 3);
    assert(ret == 3);
    assert(strcmp(buffer, "tt") == 0);

    fd2 = sys_dup2(curr_scheduling_proc, fd, 1);
    assert(1 == fd2);

    ret = sys_read(curr_scheduling_proc, fd2, buffer, 4);
    assert(ret == 4);
    assert(strcmp(buffer, "ttt") == 0);

    ret = sys_close(curr_scheduling_proc, fd);
    assert(ret == 0);

    ret = sys_close(curr_scheduling_proc, fd2);
    assert(ret == 0);

    ret = sys_close(curr_scheduling_proc, fd3);
    assert(ret == EBADF);
}

int _run_unit_tests(){
    test_given_o_creat_when_open_file_should_return_0();
    test_when_creating_file_should_return_0();
    test_given_two_file_descriptors_when_dupping_file_should_behave_the_same();
}

int run_unit_tests(){

    init_bitmap();
    init_dev();
    init_fs();
    init_tty();
    init_drivers();

    mock_init_proc();

    _run_unit_tests();
    unit_test1();
    unit_test2();
    unit_test3();
    unit_test_driver();
    printf("filesystem unit test passed\n");
    return 0;
}

