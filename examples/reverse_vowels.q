// Write a function that takes a string as input and reverse only the vowels of a string.
// Example 1:
// Input: "hello"
// Output: "holle"
// Example 2:
// Input: "leetcode"
// Output: "leotcede"

fn reverse_vowels(s: !string) {
  const vowels = {'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U'};
  let l = 0;
  let r = s.len() - 1;

  while l < r {
    while l < r && !vowels.has(s[l]) => l++;
    while l < r && !vowels.has(s[r]) => r--;

    if l < r {
      s.swap(l, r); l++; r--;
    }
  }
}
