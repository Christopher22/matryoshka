/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.IO;
using System.ComponentModel;
using System.Security.Permissions;
using System.Security;
using System.Threading;
using System.Runtime.InteropServices;
using System.Runtime.ConstrainedExecution;
using Microsoft.Win32.SafeHandles;

namespace matryoshka {
    delegate void FindCallback(IntPtr path);

    /// <summary>
    /// Contains the raw c methods
    /// </summary>
    internal unsafe static class Native {
        public unsafe struct FileSystem { };
        public unsafe struct Status { };
        public unsafe struct FileHandle { };

        [DllImport("matryoshka.dll")]
        public static extern FileSystem* Load(string path, Status** status);

        [DllImport("matryoshka.dll")]
        public static extern void DestroyFileSystem(FileSystem* file_system);

        [DllImport("matryoshka.dll")]
        public static extern void DestroyStatus(Status* status);

        [DllImport("matryoshka.dll")]
        public static extern void DestroyFileHandle(FileHandle* file_handle);

        [DllImport("matryoshka.dll")]
        public static extern IntPtr GetMessage(Status* status);

        [DllImport("matryoshka.dll")]
        public static extern FileHandle* Open(FileSystem* file_system, string path, Status** status);

        [DllImport("matryoshka.dll")]
        public static extern FileHandle* Push(FileSystem* file_system, string inner_path, string path, int chunk_size, Status** status);

        [DllImport("matryoshka.dll")]
        public static extern Status* Pull(FileSystem* file_system, FileHandle* file, string path);

        [DllImport("matryoshka.dll")]
        public static extern int Find(FileSystem* file_system, IntPtr path, [MarshalAs(UnmanagedType.FunctionPtr)]FindCallback callback);

        [DllImport("matryoshka.dll")]
        public static extern int Find(FileSystem* file_system, string path, [MarshalAs(UnmanagedType.FunctionPtr)]FindCallback callback);

        [DllImport("matryoshka.dll")]
        public static extern int GetSize(FileSystem* file_system, FileHandle* file);
    }

    /// <summary>
    /// Namespace containing handlers for easy access
    /// </summary>
    namespace handles {

        /// <summary>
        /// Wraps a native file system and ensure delete.
        /// </summary>
        [SecurityPermission(SecurityAction.InheritanceDemand, UnmanagedCode = true)]
        [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
        internal class FileSystemHandle : SafeHandleZeroOrMinusOneIsInvalid {
            public unsafe FileSystemHandle(Native.FileSystem* file_system) : base(true) {
                this.SetHandle(new IntPtr((void*)file_system));
            }

            public unsafe Native.FileSystem* GetHandle() {
                if (this.IsInvalid) {
                    throw new InvalidOperationException("Handle is already closed");
                }
                return (Native.FileSystem*)this.handle.ToPointer();
            }

            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            override protected bool ReleaseHandle() {
                unsafe {
                    Native.DestroyFileSystem((Native.FileSystem*)this.handle.ToPointer());
                }
                return true;
            }
        }

        /// <summary>
        /// Wraps a native status and ensure delete.
        /// </summary>
        [SecurityPermission(SecurityAction.InheritanceDemand, UnmanagedCode = true)]
        [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
        internal class StatusHandle : SafeHandleZeroOrMinusOneIsInvalid {
            public unsafe StatusHandle(Native.Status* status) : base(true) {
                this.SetHandle(new IntPtr((void*)status));
            }

            public string GetMessage() {
                if (this.IsInvalid) {
                    return "";
                }

                unsafe {
                    Native.Status* status_pointer = (matryoshka.Native.Status*)this.handle.ToPointer();
                    return Marshal.PtrToStringAnsi(Native.GetMessage(status_pointer));
                }
            }

            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            override protected bool ReleaseHandle() {
                unsafe {
                    Native.DestroyStatus((Native.Status*)this.handle.ToPointer());
                }
                return true;
            }
        }

        /// <summary>
        /// Wraps a native file handle and ensure delete.
        /// </summary>
        [SecurityPermission(SecurityAction.InheritanceDemand, UnmanagedCode = true)]
        [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
        internal class FileHandle : SafeHandleZeroOrMinusOneIsInvalid {
            public unsafe FileHandle(Native.FileHandle* status) : base(true) {
                this.SetHandle(new IntPtr((void*)status));
            }

            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            override protected bool ReleaseHandle() {
                unsafe {
                    Native.DestroyFileHandle((Native.FileHandle*)this.handle.ToPointer());
                }
                return true;
            }

            public unsafe Native.FileHandle* GetHandle() {
                if (this.IsInvalid) {
                    throw new InvalidOperationException("Handle is already closed");
                }
                return (Native.FileHandle*)this.handle.ToPointer();
            }
        }
    }

    /// <summary>
    /// A exception thrown on operation failure.
    /// </summary>
    public class MatryoshkaException : Exception {
        internal MatryoshkaException(handles.StatusHandle handle) : base(handle.GetMessage()) { }
    }

    /// <summary>
    /// A virtual file system in a Matryoshka file.
    /// </summary>
    public class FileSystem : IDisposable {
        private handles.FileSystemHandle handle_;

        public FileSystem(string path) {
            unsafe {
                Native.Status* status;
                Native.FileSystem* file_system = Native.Load(path, &status);
                if (file_system == null) {
                    using (handles.StatusHandle handle = new handles.StatusHandle(status)) {
                        throw new MatryoshkaException(handle);
                    }
                } else {
                    handle_ = new handles.FileSystemHandle(file_system);
                }
            }
        }

        public void Dispose() {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        [SecurityPermission(SecurityAction.Demand, UnmanagedCode = true)]
        protected virtual void Dispose(bool disposing) {
            if (handle_ != null && !handle_.IsInvalid) {
                handle_.Dispose();
            }
        }

        public File Open(string path) {
            unsafe {
                Native.Status* status;
                Native.FileSystem* file_system = handle_.GetHandle();
                Native.FileHandle* file = Native.Open(file_system, path, &status);
                if (file_system == null) {
                    using (handles.StatusHandle handle = new handles.StatusHandle(status)) {
                        throw new MatryoshkaException(handle);
                    }
                } else {
                    return new File(this, file, path);
                }
            }
        }

        public File Push(string inner_path, string path, int chunk_size = -1) {
            unsafe {
                Native.Status* status;
                Native.FileSystem* file_system = handle_.GetHandle();
                Native.FileHandle* file = Native.Push(file_system, inner_path, path, chunk_size, &status);
                if (file_system == null) {
                    using (handles.StatusHandle handle = new handles.StatusHandle(status)) {
                        throw new MatryoshkaException(handle);
                    }
                } else {
                    return new File(this, file, path);
                }
            }
        }

        public List<string> Find(string path = null) {
            List<string> files = new List<string>();
            unsafe {
                Native.FileSystem* file_system = handle_.GetHandle();
                if (path == null) {
                    Native.Find(file_system, IntPtr.Zero, x => files.Add(Marshal.PtrToStringAnsi(x)));
                } else {
                    Native.Find(file_system, path, x => files.Add(Marshal.PtrToStringAnsi(x)));
                }
            }
            return files;
        }

        internal unsafe Native.FileSystem* GetHandle() {
            return handle_.GetHandle();
        }
    }

    /// <summary>
    /// A file as part of a virtual file system in Matryoshka.
    /// </summary>
    public class File {
        private handles.FileHandle handle_;
        private FileSystem parent_;
        private string path_;

        internal unsafe File(FileSystem parent, Native.FileHandle* handle, string path) {
            parent_ = parent;
            handle_ = new handles.FileHandle(handle);
            path_ = path;
        }

        public void Pull(string file) {
            unsafe {
                Native.Status* status = Native.Pull(parent_.GetHandle(), handle_.GetHandle(), file);
                if (status != null) {
                    using (handles.StatusHandle handle = new handles.StatusHandle(status)) {
                        throw new MatryoshkaException(handle);
                    }
                }
            }
        }

        public int Size {
            get {
                unsafe {
                    return Native.GetSize(parent_.GetHandle(), handle_.GetHandle());
                }
            }
        }

        public override string ToString() {
            return path_;
        }
    }

    /// <summary>
    /// An example program showing usage
    /// </summary>
    class ExampleProgram {
        static int Main(string[] args) {
            if (args.Length != 1) {
                Console.WriteLine("Usage: programm [path_to_database]");
                return 0;
            }

            matryoshka.FileSystem file_system;
            try {
                file_system = new matryoshka.FileSystem(args[0]);
            } catch (matryoshka.MatryoshkaException ex) {
                Console.WriteLine(ex.Message);
                return 1;
            }

            // Pull files
            foreach (string path in file_system.Find()) {
                // Open the file
                File file = file_system.Open(path);

                // Pull it to the file system
                file.Pull($"pulled_{file}");

                // Report it
                Console.WriteLine($"Pulled: {file} ({file.Size} bytes)");
            }

            // Push all files starting with "push_"
            foreach (string file in Directory.EnumerateFiles(".", "push_*")) {
                file_system.Push(Path.GetFileName(file), file, 64);
                Console.WriteLine($"Pushed: {file}");
            }

            return 0;
        }
    }
}