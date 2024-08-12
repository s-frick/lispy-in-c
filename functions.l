def {fun} (\ {args body} {def (head args) (\ (tail args) body)})

fun {unpack f xs} {eval (join (list f) xs)}

fun {pack f & xs} {f xs}


def {uncurry} pack
def {curry} unpack



fun {reverse xs} {
  if (== xs {}) 
    {{}} 
    {join (reverse (tail xs)) (head xs)} 
} 
fun {len xs} {
  if (== xs {}) 
    {0} 
    {+ 1 (len (tail xs))}
  }
