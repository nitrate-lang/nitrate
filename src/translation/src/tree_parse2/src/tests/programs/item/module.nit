
pub mod pub_module {  }
pro mod pro_module {  }
sec mod sec_module {  }

mod module2 {
    # This is a submodule
    sec mod [] submodule {

        # Another level of submodule
        mod [
            # This is a module attribute
            HelloWorld,
            42,
        ] subsubmodule {    }
    }
}