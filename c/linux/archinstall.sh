#!/bin/bash
set -e

cat << "EOF"
SSSSSSSSSSSSSSS                      ffffffffffffffff                               CCCCCCCCCCCCChhhhhhh                                       tttt               tttt            iiii                                       
SS:::::::::::::::S                    f::::::::::::::::f                           CCC::::::::::::Ch:::::h                                    ttt:::t            ttt:::t           i::::i                                      
S:::::SSSSSS::::::S                   f::::::::::::::::::f                        CC:::::::::::::::Ch:::::h                                    t:::::t            t:::::t            iiii                                       
S:::::S     SSSSSSS                   f::::::fffffff:::::f                       C:::::CCCCCCCC::::Ch:::::h                                    t:::::t            t:::::t                                                       
S:::::S              aaaaaaaaaaaaa    f:::::f       ffffffeeeeeeeeeeee          C:::::C       CCCCCC h::::h hhhhh         aaaaaaaaaaaaa  ttttttt:::::tttttttttttttt:::::ttttttt    iiiiiiinnnn  nnnnnnnn       ggggggggg   ggggg
S:::::S              a::::::::::::a   f:::::f           ee::::::::::::ee       C:::::C               h::::hh:::::hhh      a::::::::::::a t:::::::::::::::::tt:::::::::::::::::t    i:::::in:::nn::::::::nn    g:::::::::ggg::::g
S::::SSSS           aaaaaaaaa:::::a f:::::::ffffff    e::::::eeeee:::::ee     C:::::C               h::::::::::::::hh    aaaaaaaaa:::::at:::::::::::::::::tt:::::::::::::::::t     i::::in::::::::::::::nn  g:::::::::::::::::g
 SS::::::SSSSS               a::::a f::::::::::::f   e::::::e     e:::::e     C:::::C               h:::::::hhh::::::h            a::::atttttt:::::::tttttttttttt:::::::tttttt     i::::inn:::::::::::::::ng::::::ggggg::::::gg
   SSS::::::::SS      aaaaaaa:::::a f::::::::::::f   e:::::::eeeee::::::e     C:::::C               h::::::h   h::::::h    aaaaaaa:::::a      t:::::t            t:::::t           i::::i  n:::::nnnn:::::ng:::::g     g:::::g 
      SSSSSS::::S   aa::::::::::::a f:::::::ffffff   e:::::::::::::::::e      C:::::C               h:::::h     h:::::h  aa::::::::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g 
           S:::::S a::::aaaa::::::a  f:::::f         e::::::eeeeeeeeeee       C:::::C               h:::::h     h:::::h a::::aaaa::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g 
           S:::::Sa::::a    a:::::a  f:::::f         e:::::::e                 C:::::C       CCCCCC h:::::h     h:::::ha::::a    a:::::a      t:::::t    tttttt  t:::::t    tttttt i::::i  n::::n    n::::ng::::::g    g:::::g 
SSSSSSS     S:::::Sa::::a    a:::::a f:::::::f        e::::::::e                 C:::::CCCCCCCC::::C h:::::h     h:::::ha::::a    a:::::a      t::::::tttt:::::t  t::::::tttt:::::ti::::::i n::::n    n::::ng:::::::ggggg:::::g 
S::::::SSSSSS:::::Sa:::::aaaa::::::a f:::::::f         e::::::::eeeeeeee          CC:::::::::::::::C h:::::h     h:::::ha:::::aaaa::::::a      tt::::::::::::::t  tt::::::::::::::ti::::::i n::::n    n::::n g::::::::::::::::g 
S:::::::::::::::SS  a::::::::::aa:::af:::::::f          ee:::::::::::::e            CCC::::::::::::C h:::::h     h:::::h a::::::::::aa:::a       tt:::::::::::tt    tt:::::::::::tti::::::i n::::n    n::::n  gg::::::::::::::g 
 SSSSSSSSSSSSSSS     aaaaaaaaaa  aaaafffffffff            eeeeeeeeeeeeee               CCCCCCCCCCCCC hhhhhhh     hhhhhhh  aaaaaaaaaa  aaaa         ttttttttttt        ttttttttttt  iiiiiiii nnnnnn    nnnnnn    gggggggg::::::g 
                                                                                                                                                                                                                        g:::::g 
                                                                                                                                                                                                            gggggg      g:::::g 
                                                                                                                                                                                                            g:::::gg   gg:::::g 
                                                                                                                                                                                                             g::::::ggg:::::::g 
                                                                                                                                                                                                              gg:::::::::::::g  
                                                                                                                                                                                                                ggg::::::ggg    
                                                                                                                                                                                                                   gggggg      
EOF

echo ""
echo "=== SafeChatting — установка зависимостей (Arch Linux) ==="

if [ "$(id -u)" -ne 0 ]; then
    echo "Запусти с sudo: sudo $0"
    exit 1
fi

DEPS="gcc make openssl"
MISSING=""

for pkg in $DEPS; do
    if pacman -Qi "$pkg" 2>/dev/null | grep -q "Installed On"; then
        echo "  ✓ $pkg уже установлен"
    else
        echo "  ✗ $pkg будет установлен"
        MISSING="$MISSING $pkg"
    fi
done

if [ -n "$MISSING" ]; then
    echo ""
    echo "Установка:$MISSING"
    pacman -S --noconfirm $MISSING
    echo ""
    echo "Готово! Зависимости установлены."
else
    echo ""
    echo "Всё уже установлено."
fi
