
pub mod pub_module {  }
pro mod pro_module {  }
sec mod sec_module {  }

mod module2 {
    # This is a submodule
    sec mod [] submodule {

        # Another level of submodule
        mod [
            false, 
            42
        ] subsubmodule {    }
    }
}