package projetoedd;

import javax.swing.*;

public class main {
    public static void main(String[] args) {
       System.setProperty("jna.library.path", "C:\\Users\\carol\\OneDrive\\Documentos\\NetBeansProjects\\PROJETOEDD");
        SwingUtilities.invokeLater(() -> {
            JFrame frame = new JFrame("Banco de Dados - Projeto EDD");
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setSize(500, 400);
            frame.setLocationRelativeTo(null);

            // Inicia exibindo a tela de Bem-vindo
            frame.setContentPane(new NewJPanel());
            frame.setVisible(true);
        });
    }
  
}
