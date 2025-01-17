
# JackTokenizer: JackTokenizer.c
# 	gcc JackTokenizer.c -std=c11 -o JackTokenizer
# 	chmod +x JackTokenizer

# CompilationEngine: CompilationEngine.c JackTokenizer.c
# 	gcc CompilationEngine.c -std=gnu11 -Wno-int-conversion -o CompilationEngine
# 	chmod +x CompilationEngine

JackAnalyzer: JackAnalyzer.c JackTokenizer.c CompilationEngine.c
	gcc JackAnalyzer.c -std=gnu11 -Wno-int-conversion -o JackAnalyzer
	chmod +x JackAnalyzer

