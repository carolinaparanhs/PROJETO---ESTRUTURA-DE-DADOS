/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Interface.java to edit this template
 */
package projetoedd;
import com.sun.jna.Library;
import com.sun.jna.Native;

public interface BTreeLib extends Library {
    BTreeLib INSTANCE = (BTreeLib) Native.load("btree", BTreeLib.class);
    void btree_insert(int key);
    int btree_search(int key);
}


