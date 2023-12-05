# What is this?

An example program to demonstrate possibly memory leaks in Zenoh-C.

# Build

This was tested and built on Ubuntu 22.04 with cargo 1.72 and Rust 1.72.

## Build zenoh-c

git clone https://github.com/eclipse-zenoh/zenoh-c.git
pushd zenoh-c
git reset --hard 10a198a
popd
mkdir build-zenoh-c
pushd build-zenoh-c
cmake ../zenoh-c -DCMAKE_INSTALL_PREFIX=../install
cmake --build . --config Release --target install
popd

## Build example program

mkdir build
pushd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Debug
make -j10
popd

# Run

valgrind --leak-check=full ./build/zenoh-leak

# Analysis

Here are a couple of the types of leaks reported by valgrind.
These are only a subset; run the command above to get all of them.

## Leak 1

==3120319== 1 bytes in 1 blocks are possibly lost in loss record 1 of 240
==3120319==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==3120319==    by 0x4C9D719: zenoh::net::routing::resource::Resource::new (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4C8107F: zenoh::net::routing::resource::Resource::make_resource (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4C7A15C: <zenoh::net::routing::face::Face as zenoh_transport::primitives::Primitives>::send_declare (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4CDAF67: <zenoh_transport::primitives::demux::DeMux<P> as zenoh_transport::TransportPeerEventHandler>::handle_message (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4CD6A58: <zenoh::net::routing::router::LinkStateInterceptor as zenoh_transport::TransportPeerEventHandler>::handle_message (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4CFB42D: <zenoh::net::runtime::RuntimeSession as zenoh_transport::TransportPeerEventHandler>::handle_message (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4EB0725: zenoh_transport::unicast::universal::rx::<impl zenoh_transport::unicast::universal::transport::TransportUnicastUniversal>::trigger_callback (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4EAC014: <core::pin::Pin<P> as core::future::future::Future>::poll (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4EA9946: async_task::raw::RawTask<F,T,S,M>::run (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49FE1E6: async_global_executor::threading::thread_main_loop (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49FD5E5: std::sys_common::backtrace::__rust_begin_short_backtrace (in zenoh_leak/install/lib/libzenohc.so)

As reported by Zettascale, this kind of leak is due to global thread initialization.
These aren't particularly worrisome, though they make further analysis a bit annoying.

## Leak 2

==3120319== 3 bytes in 1 blocks are possibly lost in loss record 3 of 240
==3120319==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==3120319==    by 0x492411A: zenoh_transport::manager::TransportManagerBuilder::from_config::{{closure}} (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4912721: <async_std::task::builder::SupportTaskLocals<F> as core::future::future::Future>::poll (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49BE250: z_open (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x1091E2: main (zenoh-leak.cpp:11)

This one seems much more like a real leak.  `z_open` was called, allocated some memory, but that memory seems like it is never being freed.

## Leak 3

==3120319== 22 bytes in 1 blocks are possibly lost in loss record 28 of 240
==3120319==    at 0x484DCD3: realloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==3120319==    by 0x4E1F36A: alloc::raw_vec::finish_grow (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4DDAA71: alloc::raw_vec::RawVec<T,A>::reserve_for_push (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4DF7907: <zenoh_link_tls::TlsConfigurator as zenoh_link_commons::ConfigurationInspector<zenoh_config::Config>>::inspect_config::{{closure}} (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49241AF: zenoh_transport::manager::TransportManagerBuilder::from_config::{{closure}} (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4912721: <async_std::task::builder::SupportTaskLocals<F> as core::future::future::Future>::poll (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49BE250: z_open (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x1091E2: main (zenoh-leak.cpp:11)

Similarly, this seems like a real leak, though I'm possibly missing something about how rust works here.

## Leak 4

==3120319== 40 bytes in 1 blocks are possibly lost in loss record 64 of 240
==3120319==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==3120319==    by 0x4C93E9D: zenoh::net::routing::queries::compute_query_route (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4C92792: zenoh::net::routing::queries::compute_query_routes_ (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4CAEC0D: <zenoh::net::routing::face::Face as zenoh_transport::primitives::Primitives>::send_close (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4CFC06F: <zenoh::net::runtime::RuntimeSession as zenoh_transport::TransportPeerEventHandler>::closing (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4EA64F9: zenoh_transport::unicast::universal::transport::TransportUnicastUniversal::delete::{{closure}} (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x4EA56C6: <zenoh_transport::unicast::universal::transport::TransportUnicastUniversal as zenoh_transport::unicast::transport_unicast_inner::TransportUnicastTrait>::close::{{closure}} (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49113BB: <async_std::task::builder::SupportTaskLocals<F> as core::future::future::Future>::poll (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x49C02DC: z_close (in zenoh_leak/install/lib/libzenohc.so)
==3120319==    by 0x1091F2: main (zenoh-leak.cpp:13)

Interestingly this seems to be a leak during `z_close`, which is unusual.
