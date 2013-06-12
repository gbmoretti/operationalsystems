require 'matrix'

class Matriz

 def initialize(qtd,tam,arq="matriz.txt")
  puts "Gerando #{qtd} matrizes #{tam}x#{tam} e guardando em #{arq}"
  @@f = File.new(arq, "w+")
  @@f << tam << "\n" << qtd << "\n"
  qtd.times { Matriz.gerar(tam) }
  puts "Pronto!"
end

 def self.gerar(tam)
    rows = Array.new
    tam.times do
      row = Array.new
      tam.times do
        row.push 100 - rand(200)
      end
      rows.push row
    end
    m = Matrix.columns(rows)
    tam.times do |n|
      m.row(n) do |i|
        @@f << "#{i} "
      end
      @@f << "\n"
    end

 end
end

if(ARGV.size > 1 && ARGV[0].to_i > 0 && ARGV[1].to_i > 2)
  Matriz.new(ARGV[0].to_i,ARGV[1].to_i) if !ARGV[2]
  Matriz.new(ARGV[0].to_i,ARGV[1].to_i,ARGV[2]) if ARGV[2]
else
  puts "Argumentos: <QTD> <TAM> <ARQ>"
end

