import java.util.LinkedList;
import java.util.Queue;

/*
	d为当前遍历节点的深度,根节点深度为0
	键的长度 = 键所在节点的深度d:""在根节点,"a"在深度为1的第一个节点
	大部分方法均基于 (Node x, int d, String s) 为参数的递归方法
*/
public class TrieST<Value> {
	private static final int R = 256;
	private Node root;
	private int size = 0;

	private static class Node {
		private Object val;
		private Node[] next = new Node[R];
	}

	public TrieST() {}

	public void put(String key, Value val) {
		root = put(root, key, val, 0);
		size++;
	}

	private Node put(Node x, String key, Value val, int d) {
		if(x == null) x = new Node();
		if(key.length() == d) {
			x.val = val;
			return x;
		}
		char c = key.charAt(d);
		x.next[c] = put(x.next[c], key, val, d+1);
		return x;
	}

	public Value get(String key) {
		Node x = get(root, key, 0);
		if(x == null) return null;
		return (Value) x.val;
	}

	private Node get(Node x, String key, int d) {
		if(x == null) return null;
		if(key.length() == d) return x;
		char c = key.charAt(d);
		return get(x.next[c], key, d+1);
	}

	public void delete(String key) {
		root = delete(root, key, 0);
		size--;
	}

	// 递归地删除节点
	// 将要删除的节点的val置位null
	// 如果当前节点的子节点均为null而且当前节点val为null,则将当前节点删除(返回给父节点null)
	// 递归地自底向上删除
	private Node delete(Node x, String key, int d) {
		if(x == null) return null;
		if(d != key.length()) {
			char c = key.charAt(d);
			x.next[c] = delete(x.next[c], key, d+1);
		}
		else if(d == key.length()) {
			x.val = null;
		}
		if(x.val != null) return x;

		for(int i = 0; i < R; i++) {
			if(x.next[i] != null) return x;
		}
		return null;
	}

	public boolean contains(String key) {
		return get(root, key, 0) != null;
	}

	public boolean isEmpty() {
		return size == 0;
	}

	// s的前缀中最长的键
	public String longestPrefixOf(String s) {
		int length = search(root, s, 0, 0);
		return s.substring(0, length);
	}

	// 从当前节点开始递归查找满足s前缀最长的长度
	// length 为已经找到的最长前缀长度
	// d为当前查找的深度
	private int search(Node x, String s, int d, int length) {
		if(x == null) return length;
		if(x.val != null) length = d;
		if(d == s.length()) return length; // s遍历完了

		char c = s.charAt(d);
		return search(x.next[c], s, d+1, length);
	}



	// 所有以pre为前缀的键
	// 先找到pre键的节点,从这个节点开始递归地添加所有满足的后续键
	public Iterable<String> keyWithPrefix(String pre) {
		Queue<String> q = new LinkedList<String>();
		collect(get(root, pre, 0), pre, q);
		return q;
	}

	// 和pat匹配的键(.能够匹配任意字符)
	// 从根节点开始,递归地添加所有满足模式的键
	public Iterable<String> KeysThatMatch(String pat) {
		Queue<String> q = new LinkedList<String>();
		collect(root, "", pat, q);
		return q;
	}

	// 键值对的数量
	public int size() {
		return size;
	}

	// 符号表中的所有键
	// 前缀为""的所有键即可
	public Iterable<String> keys() {
		return keyWithPrefix("");
	}

	// 从满足pre的节点开始递归地添加所有满足pre的后续的键
	private void collect(Node x, String pre, Queue<String> q) {
		if(x == null) return;
		if(x.val != null) q.add(pre);
		for(char c = 0; c < R; c++) {
			collect(x.next[c], pre + c, q);
		}
	}

	// 当前键(pre),将满足pat的键添加到队列
	private void collect(Node x, String pre, String pat, Queue<String> q) {
		int d = pre.length();
		if(x == null) return;
		if(d == pat.length() && x.val != null) q.add(pre);
		if(d == pat.length()) return;  // 模式已经匹配完了

		char next = pat.charAt(d); // 下一个要匹配的字符
		for(char c = 0; c < R; c++) {
			if(next == c || next == '.') {
				collect(x.next[c], pre + c, pat, q);
			}	
		}
	}
	public static void main(String[] args) {
		TrieST<Integer> tst = new TrieST<Integer>();
		tst.put("1", 1);
		tst.put("12", 2);
		tst.put("3", 3);
		tst.put("34", 4);
		tst.put("5", 5);

		for(String s: tst.keys()) {
			System.out.println(s);
			tst.delete(s);
			System.out.println(tst.size());
		}
		System.out.println(tst.isEmpty());
	}
}