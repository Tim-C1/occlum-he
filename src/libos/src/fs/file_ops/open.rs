use super::*;

fn do_open(path: &str, flags: u32, mode: u32) -> Result<FileDesc> {
    let current_ref = process::get_current();
    let mut proc = current_ref.lock().unwrap();

    let file = proc.open_file(path, flags, mode)?;
    let file_ref: Arc<Box<dyn File>> = Arc::new(file);

    let fd = {
        let creation_flags = CreationFlags::from_bits_truncate(flags);
        proc.get_files()
            .lock()
            .unwrap()
            .put(file_ref, creation_flags.must_close_on_spawn())
    };
    Ok(fd)
}

pub fn do_openat(dirfd: DirFd, path: &str, flags: u32, mode: u32) -> Result<FileDesc> {
    info!(
        "openat: dirfd: {:?}, path: {:?}, flags: {:#o}, mode: {:#o}",
        dirfd, path, flags, mode
    );
    if Path::new(path).is_absolute() {
        // Path is absolute, so dirfd is ignored
        return Ok(do_open(path, flags, mode)?);
    }
    let path = match dirfd {
        DirFd::Fd(dirfd) => {
            let dir_path = get_dir_path(dirfd)?;
            dir_path + "/" + path
        }
        DirFd::Cwd => path.to_owned(),
    };
    do_open(&path, flags, mode)
}
