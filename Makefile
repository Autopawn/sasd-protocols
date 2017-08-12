ballarmada:
	cd src/ballarmada; rm *.class || true
	cd src/ballarmada; javac BAEvent.java
	cd src/ballarmada; javac BABall.java
	cd src/ballarmada; javac BAState.java
	cd src/ballarmada; javac BATrace.java
	cd src/ballarmada; javac BAGameplay.java
	cd src/ballarmada; javac BAGameTest.java
run:
	cd src/ballarmada; java BAGameTest
document:
	cd tex; pdflatex main.tex
	cd tex; bibtex main
	cd tex; pdflatex main.tex
	cd tex; pdflatex main.tex
	mv tex/main.pdf main.pdf
