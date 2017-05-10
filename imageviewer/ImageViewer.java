/*
 * Gary Cornell and Cay S. Horstmann, Core Java (Book/CD-ROM)
 * Published By SunSoft Press/Prentice-Hall
 * Copyright (C) 1996 Sun Microsystems Inc.
 * All Rights Reserved. ISBN 0-13-565755-5
 *
 * Permission to use, copy, modify, and distribute this 
 * software and its documentation for NON-COMMERCIAL purposes
 * and without fee is hereby granted provided that this 
 * copyright notice appears in all copies. 
 * 
 * THE AUTHORS AND PUBLISHER MAKE NO REPRESENTATIONS OR 
 * WARRANTIES ABOUT THE SUITABILITY OF THE SOFTWARE, EITHER 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. THE AUTHORS
 * AND PUBLISHER SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED 
 * BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING 
 * THIS SOFTWARE OR ITS DERIVATIVES.
 */
 
/**
 * @version 1.00 07 Feb 1996 
 * @author Cay Horstmann
 */

import java.awt.*;
import java.awt.image.*;
import java.net.*;
import java.io.*;


public class ImageViewer extends Frame {  

  private Image image = null;

  public ImageViewer() {  

	setTitle("ImageViewer");

      MenuBar mbar = new MenuBar();
      Menu m = new Menu("File");
      m.add(new MenuItem("Open"));            
      m.add(new MenuItem("Exit"));            
      mbar.add(m);
      setMenuBar(mbar);
   }

   public boolean handleEvent(Event evt) {  

	if (evt.id == Event.WINDOW_DESTROY) 
		System.exit(0);

      return super.handleEvent(evt);
   }

   public boolean action(Event evt, Object arg) {  

     if (arg.equals("Open")) {  

	FileDialog d = new FileDialog(this,
            "Open image file", FileDialog.LOAD);

         d.setFile("*.gif");
         d.show();

         String f = d.getFile();

         if (f != null)
            image = Toolkit.getDefaultToolkit().getImage(f);

         repaint();

      } else if (arg.equals("Exit")) 
		System.exit(0);

      else 
		return false;

      return true;
   }
   
   public void paint(Graphics g) {  

     if (image != null)   
       g.drawImage(image, 0, 0, this);

   }

   public static void main(String args[]) {  

	Frame f = new ImageViewer();


      f.resize(300, 200);
      f.show();
   }

}





