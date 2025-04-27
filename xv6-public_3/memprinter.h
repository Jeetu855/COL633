struct proc_page_info {
    int pid;
    int pages;
  };
  
  extern struct proc_page_info pp_info[NPROC];
  extern int pp_count;
  void collect_proc_pages(void);
  