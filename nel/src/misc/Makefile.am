#
# $Id: Makefile.am,v 1.70 2003/11/17 10:31:01 lecroart Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = aabbox.cpp                         \
			algo.cpp 			   \
                        async_file_manager.cpp             \
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
                        diff_tool.cpp                      \
                        displayer.cpp                      \
                        eid_translator.cpp                 \
                        entity_id.cpp                      \
                        eval_num_expr.cpp                  \
                        event_emitter.cpp                  \
                        event_listener.cpp                 \
                        event_server.cpp                   \
                        events.cpp                         \
                        fast_mem.cpp                       \
                        file.cpp                           \
                        game_device_events.cpp             \
                        geom_ext.cpp                       \
                        gtk_displayer.cpp                  \
			heap_allocator.cpp		   \
                        heap_memory.cpp                    \
			hierarchical_timer.cpp             \
                        i_xml.cpp                          \
                        i18n.cpp                           \
                        input_device.cpp                   \
                        input_device_server.cpp            \
                        line.cpp                           \
                        log.cpp                            \
                        matrix.cpp                         \
                        mem_displayer.cpp                  \
                        mem_stream.cpp                     \
                        mutex.cpp                          \
			noise_value.cpp                    \
                        o_xml.cpp                          \
                        p_thread.cpp                       \
                        path.cpp                           \
                        plane.cpp                          \
                        polygon.cpp                        \
			progress_callback.cpp		   \
                        quad.cpp                           \
                        quat.cpp                           \
                        reader_writer.cpp                  \
                        rect.cpp                           \
                        report.cpp                         \
                        rgba.cpp                           \
                        shared_memory.cpp                  \
                        sheet_id.cpp                       \
                        smart_ptr.cpp                      \
                        stop_watch.cpp                     \
                        stream.cpp                         \
                        string_id_array.cpp                \
			string_mapper.cpp		   \
                        system_info.cpp                    \
                        task_manager.cpp                   \
			tds.cpp				   \
                        time_nl.cpp                        \
                        triangle.cpp                       \
                        uv.cpp                             \
                        variable.cpp                       \
                        value_smoother.cpp                 \
                        vector.cpp                         \
                        vector_2d.cpp                      \
                        vector_2f.cpp                      \
                        vector_h.cpp                       \
                        vectord.cpp                        \
                        win_event_emitter.cpp              \
                        win_thread.cpp                     \
                        win_displayer.cpp                  \
                        window_displayer.cpp               \
                        words_dictionary.cpp

noinst_HEADERS        = di_game_device.h                   \
                        di_keyboard_device.h               \
                        di_mouse_device.h                  \
                        stdmisc.h

libnelmisc_la_LIBADD  = config_file/libconfig.la -lc -lpthread

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

