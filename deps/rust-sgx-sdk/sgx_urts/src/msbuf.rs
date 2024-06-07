// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License..

use libc::{self, c_int, c_void, off64_t, size_t};
use std::ptr;

#[no_mangle]
pub extern "C" fn u_read_shared_buf(
    base: *const c_void,
    data: *mut c_void,
    count: size_t,
    offset: off64_t,
) -> c_int {
    if base.is_null() || data.is_null() || count == 0 {
        return libc::EINVAL;
    }

    unsafe {
        let src = (base as *const u8).add(offset as usize);
        ptr::copy_nonoverlapping(src, data as *mut u8, count);
    }
    0
}

#[no_mangle]
pub extern "C" fn u_write_shared_buf(
    base: *mut c_void,
    data: *const c_void,
    count: size_t,
    offset: off64_t,
) -> c_int {
    if base.is_null() || data.is_null() || count == 0 {
        return libc::EINVAL;
    }

    unsafe {
        let dest = (base as *mut u8).add(offset as usize);
        ptr::copy_nonoverlapping(data as *const u8, dest, count);
    }
    0
}
