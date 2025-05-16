use proc_macro::TokenStream;
use quote::quote;
use syn::Data;

#[proc_macro_derive(ExtendEnum)]
pub fn extend_enum_derive(input: TokenStream) -> TokenStream {
    let ast = syn::parse(input).unwrap();
    // println!("{:#?}", ast);
    impl_extend_enum_macro(&ast)
}

fn impl_extend_enum_macro(ast: &syn::DeriveInput) -> TokenStream {
    let name = &ast.ident;
    let str_name = name.to_string();
    let Data::Enum(data) = &ast.data else {
        return Default::default();
    };

    let cases = data.variants.iter();
    let str_cases = cases.clone().map(|v| v.ident.to_string().to_lowercase().replace("_", "-"));

    let cases_2 = cases.clone();
    let str_cases_2 = str_cases.clone();

    let gen_code = quote! {
        impl FromStr for #name {
            type Err = String;

            fn from_str(s: &str) -> Result<Self, Self::Err> {
                match s {
                    #(#str_cases => Ok(#name::#cases),)*
                    _ => Err(format!("\"{}\" cannot be converted to a {}.", s, #str_name).into()),
                }
            }
        }

        impl ::core::fmt::Display for #name {
            fn fmt(&self, f: &mut ::core::fmt::Formatter) -> ::core::fmt::Result {
                let s = match self {
                    #(#name::#cases_2 => #str_cases_2,)*
                };
                write!(f, "{}", s)
            }
        }
    };

    // println!("{}", gen_code);
    gen_code.into()
}
