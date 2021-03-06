#include "application.hpp"

#include <morda/widgets/group/tabbed_book.hpp>
#include <morda/widgets/label/text.hpp>

#include <utki/unicode.hpp>

#include <papki/fs_file.hpp>

#include "editor_page.hpp"

using namespace cod;

namespace{
std::shared_ptr<morda::tab> inflate_tab(std::shared_ptr<morda::tabbed_book> tb, const std::string& name){
	auto t = tb->context->inflater.inflate_as<morda::tab>(R"(
		@tab{
			@row{
				@text{
					id{text}
					text{cube}
				}
				@push_button{
					id{close_button}
					@image{
						layout{
							dx { 8dp }
							dy { 8dp }
						}
						image{morda_img_close}
					}
				}
			}
		}
	)");
	t->get_widget_as<morda::text>("text").set_text(name);

	auto& close_btn = t->get_widget_as<morda::push_button>("close_button");
	
	close_btn.click_handler = [
			tabbed_book_wp = utki::make_weak(tb),
			tab_wp = utki::make_weak(t)
		](morda::push_button& btn)
	{
		auto tb = tabbed_book_wp.lock();
		ASSERT(tb)

		auto t = tab_wp.lock();
		ASSERT(t)

		tb->tear_out(*t);
	};
	return t;
}
}

application::application(command_line_arguments&& cla) :
		mordavokne::application(
				"cod",
				[](){
					return mordavokne::window_params(r4::vector2<unsigned>(1024, 768));
				}()
			),
		cla(std::move(cla))
{
	this->gui.initStandardWidgets(*this->get_res_file());
	
	this->gui.context->inflater.register_widget<code_edit>("code_edit");
	this->gui.context->inflater.register_widget<file_tree>("file_tree");

	this->gui.context->loader.mount_res_pack(*this->get_res_file("res/"));
	
	auto c = this->gui.context->inflater.inflate(
			*this->get_res_file("res/main.gui")
		);

	auto& tb = c->get_widget_as<morda::tabbed_book>("tabbed_book");

	auto& ft = c->get_widget_as<file_tree>("file_tree");
	ft.file_select_handler = [tb = utki::make_shared_from(tb)](std::string file_name){
		if(papki::is_dir(file_name)){
			return;
		}

		papki::fs_file f(file_name);
		auto t = f.load();

		auto p = std::make_shared<editor_page>(tb->context, treeml::forest());
		p->set_text(utki::to_utf32(std::string(reinterpret_cast<char*>(t.data()), t.size())));

		tb->add(
				inflate_tab(tb, f.not_dir()),
				p
			);
		
		// std::cout << "file = " << file_name << '\n';
	};
	
	this->gui.set_root(std::move(c));
}
