#
# $Id: Makefile.am,v 1.63 2002/12/09 11:20:43 coutelas Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = aabbox.cpp                         \
			algo.cpp 			   \
			big_file.cpp                       \
                        bit_mem_stream.cpp                 \
                        bit_set.cpp                        \
                        bitmap.cpp                         \
                        block_memory.cpp                   \
                        bsphere.cpp                        \
			buf_fifo.cpp                       \
                        class_id.cpp                       \
                        class_registry.cpp                 \
                        command.cpp                        \
                        common.cpp                         \
                        cpu_info.cpp                       \
                        debug.cpp                          \
                        displayer.cpp                      \
                        event_emitter.cpp                  \
                        event_listener.cpp                 \
                        event_server.cpp                   \
                        events.cpp                         \
                        file.cpp                           \
                        geom_ext.cpp                       \
			heap_allocator.cpp		   \
                        heap_memory.cpp                    \
			hierarchical_timer.cpp             \
                        i_xml.cpp                          \
                        i18n.cpp                           \
                        line.cpp                           \
                        log.cpp                            \
                        matrix.cpp                         \
                        mem_stream.cpp                     \
                        mutex.cpp                          \
                        o_xml.cpp                          \
                        p_thread.cpp                       \
                        path.cpp                           \
                        plane.cpp                          \
                        polygon.cpp                        \
			progress_callback.cpp		   \
                        quad.cpp                           \
                        quat.cpp                           \
                        rect.cpp                           \
                        rgba.cpp                           \
                        shared_memory.cpp                  \
                        smart_ptr.cpp                      \
                        stop_watch.cpp                     \
                        stream.cpp                         \
                        string_id_array.cpp                \
                        system_info.cpp                    \
                        task_manager.cpp                   \
			tds.cpp				   \
                        time_nl.cpp                        \
                        triangle.cpp                       \
                        uv.cpp                             \
                        value_smoother.cpp                 \
                        vector.cpp                         \
                        vector_2d.cpp                      \
                        vector_2f.cpp                      \
                        vector_h.cpp                       \
                        vectord.cpp                        \
                        win_event_emitter.cpp              \
                        win_thread.cpp                     \
                        reader_writer.cpp                  \
                        entity_id.cpp                      \
                        win_displayer.cpp                  \
                        gtk_displayer.cpp                  \
                        window_displayer.cpp               \
                        mem_displayer.cpp                  \
                        game_device_events.cpp             \
                        input_device.cpp                   \
                        input_device_server.cpp            \
                        fast_mem.cpp                       \
                        sheet_id.cpp                       \
                        eval_num_expr.cpp                  \
                        report.cpp                         \
                        async_file_manager.cpp


noinst_HEADERS        = di_game_device.h                   \
                        di_keyboard_device.h               \
                        di_mouse_device.h                  \
                        stdmisc.h

libnelmisc_la_LIBADD  = config_file/libconfig.la -lc -lpthread

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

